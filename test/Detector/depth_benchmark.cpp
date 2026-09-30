// Configure with cmake -S test/Detector -B /tmp/openkai-depth-tests -DCMAKE_BUILD_TYPE=Release.
// Example: depth_benchmark --model yolo26n-depth.onnx --threads 1 --graph all --iterations 10
#include "Detector/_YOLO26depthEstONNX.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace kai;

namespace
{
    using Clock = std::chrono::steady_clock;

    void require(bool condition, const string &message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    struct Options
    {
        string model;
        string graph = "default";
        string stage = "all";
        int threads = 0;
        int cvThreads = 1;
        int warmup = 2;
        int iterations = 10;
    };

    void usage(const char *program)
    {
        std::cout << "Usage: " << program << " --model FILE [--threads N] [--cv-threads N]\n"
                  << "       [--graph default|disabled|basic|extended|all] [--warmup N] [--iterations N]\n"
                  << "       [--stage all|raw|estimate|pcl|detect]\n"
                  << "Uses synthetic 640x480 BGR input and the model's static input size (768x768 if dynamic).\n"
                  << "PCL uses stride 2 and RGB colors. Timings exclude model loading, input acquisition and warmup.\n"
                  << "detect reports RGBD publication/enqueue and completion through matching asynchronous PCL output separately.\n"
                  << "ONNX Runtime defaults to automatic threading (0); OpenCV defaults to one worker.\n";
    }

    GraphOptimizationLevel graphLevel(const string &name)
    {
        if (name == "disabled") return ORT_DISABLE_ALL;
        if (name == "basic") return ORT_ENABLE_BASIC;
        if (name == "extended") return ORT_ENABLE_EXTENDED;
        if (name == "all") return ORT_ENABLE_ALL;
        throw std::runtime_error("Invalid graph optimization: " + name);
    }

    Options arguments(int argc, char **argv)
    {
        Options options;
        for (int i = 1; i < argc; ++i)
        {
            const string key = argv[i];
            require(i + 1 < argc, "Missing value for " + key);
            const string value = argv[++i];
            if (key == "--model") options.model = value;
            else if (key == "--threads") options.threads = std::stoi(value);
            else if (key == "--cv-threads") options.cvThreads = std::stoi(value);
            else if (key == "--graph") options.graph = value;
            else if (key == "--warmup") options.warmup = std::stoi(value);
            else if (key == "--iterations") options.iterations = std::stoi(value);
            else if (key == "--stage") options.stage = value;
            else throw std::runtime_error("Unknown argument: " + key);
        }
        require(!options.model.empty(), "--model is required");
        require(options.threads >= 0 && options.cvThreads >= 0 && options.warmup >= 0 && options.iterations > 0,
                "Thread counts and warmup must be nonnegative; iterations must be positive");
        require(options.stage == "all" || options.stage == "raw" || options.stage == "estimate" ||
                    options.stage == "pcl" || options.stage == "detect", "Invalid stage: " + options.stage);
        if (options.graph != "default") graphLevel(options.graph);
        return options;
    }

    class DepthProbe : public _YOLO26depthEstONNX
    {
    public:
        using _YOLO26depthEstONNX::detect;
        using _YOLO26depthEstONNX::estimateDepth;
        using _YOLO26depthEstONNX::makePointCloud;

        bool configure(const Options &options)
        {
            m_fModel = options.model;
            m_nThread = options.threads;
            m_vRangeD = Vector2f(0.1f, 50.f);
            m_vFocal = Vector2f(600.f, 600.f);
            m_vPrincipal = Vector2f(319.5f, 239.5f);
            m_vSizeCalib = Vector2i(640, 480);
            m_nPCLstep = 2;
            m_bPCL = m_bPCLrgb = true;
            if (!loadModel()) return false;
            if (options.graph != "default")
            {
                // loadModel sets the production graph level. Recreate only for an
                // explicit comparison; leave model validation and I/O names intact.
                m_pSession.reset();
                m_sessionOptions.SetGraphOptimizationLevel(graphLevel(options.graph));
                m_pSession = std::make_unique<Ort::Session>(m_env, m_fModel.c_str(), m_sessionOptions);
            }
            return true;
        }

        void outputs(RGBframe &input, RGBDframe &rgbd, RGBframe &depth, PCLframe &points)
        {
            m_pRGBin = &input;
            m_pDout = &rgbd;
            m_pDepthOut = &depth;
            m_pPCLout = &points;
            if (!m_pT) m_pT = new _Thread();
        }

        bool startPointCloudWorker()
        {
            // Start the production worker with an empty input, then join only
            // inference so the benchmark can time each explicit detect() call.
            if (!m_pTpp) m_pTpp = new _Thread();
            if (!start()) return false;
            m_pT->join();
            return true;
        }

        Vector2i inputSize() const { return m_vModelInputSize; }

        void prepareRawInput(const Mat &rgb)
        {
            const int w = m_vModelInputSize.x(), h = m_vModelInputSize.y();
            const double gain = std::min(double(w) / rgb.cols, double(h) / rgb.rows);
            const int rw = std::max(1, std::min(w, static_cast<int>(std::nearbyint(rgb.cols * gain))));
            const int rh = std::max(1, std::min(h, static_cast<int>(std::nearbyint(rgb.rows * gain))));
            const int left = (w - rw) / 2, top = (h - rh) / 2;
            Mat resized, padded;
            cv::resize(rgb, resized, Size(rw, rh), 0, 0, INTER_LINEAR);
            cv::copyMakeBorder(resized, padded, top, h - rh - top, left, w - rw - left,
                               BORDER_CONSTANT, Scalar(114, 114, 114));
            const size_t plane = static_cast<size_t>(w) * h;
            m_tensor.resize(3 * plane);
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x)
                {
                    const Vec3b pixel = padded.at<Vec3b>(y, x);
                    const size_t index = static_cast<size_t>(y) * w + x;
                    for (int c = 0; c < 3; ++c)
                        m_tensor[c * plane + index] = pixel[m_bSwapRB ? 2 - c : c] * m_scale;
                }
            const int64_t shape[] = {1, 3, h, w};
            m_image = Ort::Value::CreateTensor<float>(m_memoryInfo, m_tensor.data(), m_tensor.size(), shape, 4);
        }

        void runRaw()
        {
            const char *inputs[] = {m_inputName.c_str()}, *outputs[] = {m_outputName.c_str()};
            auto result = m_pSession->Run(Ort::RunOptions{nullptr}, inputs, &m_image, 1, outputs, 1);
            require(result.size() == 1 && result[0].IsTensor(), "Raw inference returned no depth tensor");
        }

    private:
        vector<float> m_tensor;
        Ort::Value m_image{nullptr};
    };

    template <class Before, class Operation, class After>
    void measure(const string &name, const Options &options, Before before, Operation operation, After after)
    {
        vector<double> samples;
        samples.reserve(options.iterations);
        for (int i = 0; i < options.warmup + options.iterations; ++i)
        {
            before();
            const auto start = Clock::now();
            operation();
            const auto end = Clock::now();
            after();
            if (i >= options.warmup)
                samples.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        }
        std::sort(samples.begin(), samples.end());
        const size_t middle = samples.size() / 2;
        const double median = samples.size() % 2 ? samples[middle] : (samples[middle - 1] + samples[middle]) / 2;
        const size_t p90 = static_cast<size_t>(std::ceil(samples.size() * .9)) - 1;
        std::cout << std::left << std::setw(25) << name << std::right << std::fixed << std::setprecision(3)
                  << " median_ms=" << median << " p90_ms=" << samples[p90]
                  << " min_ms=" << samples.front() << " max_ms=" << samples.back()
                  << " median_fps=" << 1000 / median << std::endl;
    }
}

int main(int argc, char **argv)
{
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;
    FLAGS_minloglevel = 2;
    if (argc == 2 && string(argv[1]) == "--help")
    {
        usage(argv[0]);
        return 0;
    }
    try
    {
        const Options options = arguments(argc, argv);
        cv::setNumThreads(options.cvThreads);
        DepthProbe detector;
        require(detector.configure(options), "Could not load model: " + options.model);
        const Vector2i size = detector.inputSize();
        std::cout << "model=" << options.model << " onnxruntime=" << OrtGetApiBase()->GetVersionString()
                  << " threads=" << options.threads << " cv_threads=" << cv::getNumThreads()
                  << " graph=" << options.graph << " input=" << size.x() << "x" << size.y()
                  << " source=640x480 pcl_stride=2 warmup=" << options.warmup
                  << " iterations=" << options.iterations << " build="
#ifdef NDEBUG
                  << "optimized (NDEBUG)"
#else
                  << "Debug"
#endif
                  << std::endl;
        Mat rgb(480, 640, CV_8UC3);
        for (int y = 0; y < rgb.rows; ++y)
            for (int x = 0; x < rgb.cols; ++x)
                rgb.at<Vec3b>(y, x) = Vec3b(x % 256, y % 256, (x + y) % 256);
        const auto selected = [&](const string &name) { return options.stage == "all" || options.stage == name; };
        const auto nothing = [] {};
        if (selected("raw"))
        {
            detector.prepareRawInput(rgb);
            measure("onnx_run", options, nothing, [&] { detector.runRaw(); }, nothing);
        }
        Mat depth;
        if (selected("estimate"))
            measure("estimate_depth", options, nothing,
                    [&] { require(detector.estimateDepth(rgb, depth), "Depth inference failed"); }, nothing);
        if (selected("pcl"))
        {
            if (depth.empty()) require(detector.estimateDepth(rgb, depth), "Depth inference failed");
            size_t count = 0;
            measure("point_cloud_stride2", options, nothing, [&] {
                // The point-cloud worker allocates a vector per frame; include that cost.
                vector<GEOMETRY_POINT> cloud;
                detector.makePointCloud(rgb, depth, 1, cloud);
                count = cloud.size();
            }, nothing);
            std::cout << "point_count=" << count << std::endl;
        }
        if (selected("detect"))
        {
            RGBframe input, depthOutput;
            RGBDframe rgbdOutput;
            PCLframe pointOutput;
            detector.outputs(input, rgbdOutput, depthOutput, pointOutput);
            struct StopWorker
            {
                DepthProbe &detector;
                ~StopWorker() { detector.stop(); }
            } stopWorker{detector}; // Join before the local output frames die, including on failure.
            require(detector.startPointCloudWorker(), "Could not start the native point-cloud worker");
            uint64_t stamp = 0;
            const auto inputFrame = [&] { input.set(rgb, ++stamp); };
            const auto waitPointCloud = [&] {
                const auto deadline = Clock::now() + std::chrono::seconds(30);
                while (pointOutput.getTstamp() != stamp)
                {
                    require(Clock::now() < deadline, "Point-cloud worker did not publish the current frame");
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
                require(rgbdOutput.getTstamp() == stamp && depthOutput.getTstamp() == stamp,
                        "Detector did not publish matching depth outputs");
            };
            measure("detect_and_enqueue", options, inputFrame,
                    [&] { detector.detect(); }, waitPointCloud);
            measure("detect_to_point_cloud", options, inputFrame,
                    [&] { detector.detect(); waitPointCloud(); }, nothing);
        }
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        usage(argv[0]);
        return 1;
    }
    return 0;
}
