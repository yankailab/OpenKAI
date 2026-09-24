#include "../../src/Vision/RGBD/_RGBDbase.h"
#include <Scepter_api.h>
#include <Scepter_Morph_api.h>
#include <cassert>
#include <chrono>
#include <future>
#include <set>
#include <thread>
#if defined(WITH_SLAM) && defined(USE_GLIM)
#include <filesystem>
#include <glim/util/config.hpp>
#include <glim/util/logging.hpp>
#endif
// Exercise the capture and post-processing steps without starting device threads.
#define private public
#include "../../src/Vision/RGBD/_Scepter.h"
#include "../../src/Vision/Pipeline/_D2RGB.h"
#undef private
using namespace kai;
using namespace std::chrono_literals;

namespace
{
    uint16_t depth[] = {0, 65535, 500, 1000, 2000, 3000, 3500, 4000, 4500};
    uint8_t colors[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180,
                        190, 200, 210, 220, 230, 240, 250, 251, 252};
    ScStatus frameStatus = SC_OK, convertStatus = SC_OK, readyStatus = SC_OK;
    unsigned depthBytes = sizeof(depth), colorBytes = sizeof(colors);
    uint16_t colorWidth = 3;
    uint16_t frameHeight = 2;
    ScPixelFormat colorFormat = SC_PIXEL_FORMAT_BGR_888_JPEG;
    bool depthReady = true;
    std::atomic<int> conversions{0}, captures{0};
    std::atomic<bool> holdConversion{false}, converting{false};
}

// Override only frame acquisition/conversion; no camera is needed.
ScStatus scGetFrameReady(ScDeviceHandle, uint16_t, ScFrameReady *ready)
{
    ++captures;
    *ready = {};
    ready->depth = depthReady;
    ready->transformedDepth = true;
    ready->transformedColor = true;
    return readyStatus;
}
ScStatus scGetFrame(ScDeviceHandle, ScFrameType type, ScFrame *frame)
{
    *frame = {};
    frame->frameType = type;
    frame->width = 3;
    frame->height = frameHeight;
    if (type == SC_DEPTH_FRAME || type == SC_TRANSFORM_DEPTH_IMG_TO_COLOR_SENSOR_FRAME)
    {
        frame->pixelFormat = SC_PIXEL_FORMAT_DEPTH_MM16;
        frame->pFrameData = reinterpret_cast<uint8_t *>(depth);
        frame->dataLen = depthBytes;
    }
    else
    {
        frame->width = colorWidth;
        frame->pixelFormat = colorFormat;
        frame->pFrameData = colors;
        frame->dataLen = colorBytes;
    }
    return frameStatus; // Deliberately leave non-null data on SDK failure.
}
ScStatus scConvertDepthFrameToPointCloudVector(ScDeviceHandle, const ScFrame *frame, ScVector3f *points)
{
    ++conversions;
    assert(frame->pFrameData && points);
    converting = true;
    while (holdConversion) std::this_thread::yield();
    converting = false;
    if (convertStatus != SC_OK) return convertStatus;
    const auto *data = reinterpret_cast<const uint16_t *>(frame->pFrameData);
    for (size_t i = 0; i < size_t(frame->width) * frame->height; ++i)
        points[i] = {float(i), float(i), float(data[i])};
    return SC_OK;
}

class Camera : public _Scepter
{
public:
    using _Scepter::updateScRGBD;
    using _Scepter::m_bPCLframe;
    using _Scepter::m_pclStride;
    using _RGBDbase::m_bPCL;
    using _RGBDbase::m_bPCLrgb;
    Camera(_PointCloud *cloud)
    {
        m_bOpened = true;
        m_scDevHandle = reinterpret_cast<ScDeviceHandle>(1);
        m_pPCL = cloud;
        m_bRGB = false;
        m_btRGB = m_btDepth = m_bPCLrgb = true;
        m_vSizeD = Vector2i(1, 1); // Actual frames are larger than the requested size.
    }
    ~Camera() { m_scDevHandle = nullptr; m_bOpened = false; }
};
class Preview : public _D2RGB
{
public:
    explicit Preview(_RGBDbase *camera) { m_pVd = camera; }
};

int main()
{
#if defined(WITH_SLAM) && defined(USE_GLIM)
    // Scepter exports an incompatible embedded spdlog ABI. Exercise actual
    // SDK linkage and GLIM's ringbuffer formatting together: incorrect shared
    // library ordering corrupts the formatter buffer and aborts this check.
    {
        char sdkVersion[256] = {};
        assert(scGetSDKVersion(sdkVersion, sizeof(sdkVersion)) == SC_OK);
        assert(sdkVersion[0]);
        const auto profile = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() /
                             "jsonCfg/glim_scepter";
        glim::GlobalConfig::instance(profile.string());
        auto logger = glim::create_module_logger("scepter_glim_logger_regression");
        logger->info("Scepter and GLIM logger: {} {:.3f}", 7, 1.25);
        logger->flush();
        const auto lines = glim::get_ringbuffer_sink()->last_formatted();
        assert(!lines.empty());
        assert(lines.back().find("Scepter and GLIM logger: 7 1.250") != std::string::npos);
    }
#endif
    const json config = {{"class", "_Scepter"}, {"thread", {{"FPS", 30}}}, {"threadPP", {{"FPS", 30}}}};
    {
        Camera defaults(nullptr);
        defaults.setName("sampling-defaults");
        assert(defaults.init(config) && defaults.m_pclStride == 1);
        Camera sampled(nullptr);
        sampled.setName("sampling-config");
        json j = config;
        j["pclStride"] = 2;
        assert(sampled.init(j) && sampled.m_pclStride == 2);
    }
    for (const json &value : {json(0), json(-1), json(1.5), json(true), json("2"), json(nullptr), json(65536), json(4294967297ULL)})
    {
        Camera invalid(nullptr);
        invalid.setName("sampling-invalid");
        json j = config;
        j["pclStride"] = value;
        assert(!invalid.init(j));
    }

    _PointCloud cloud;
    cloud.setName("test-cloud");
    assert(cloud.init(json{{"class", "_PointCloud"}, {"thread", {{"FPS", 1}}}, {"nP", 100}}));
    Camera camera(&cloud);
    Preview preview(&camera);
    Mat image;
    std::vector<Vector3f> positions, rgb;
    uint64_t timestamp = 0;

    camera.updatePCL();
    assert(conversions == 0); // Startup: no complete frame yet.
    assert(camera.updateScRGBD());
    camera.copyMatDepth(image);
    assert(image.type() == CV_16UC1 && image.total() == 6);
    assert(image.at<uint16_t>(0, 0) == 0 && image.at<uint16_t>(0, 1) == 0);
    assert(image.at<uint16_t>(1, 2) == 3000);
    assert(depth[1] == 65535); // Never modify SDK-owned memory.
    camera.copyMatTransformedDepth(image);
    assert(image.at<uint16_t>(0, 1) == 0);
    assert(std::abs(camera.getDepthScale() - .001f) < 1e-7);

    preview.filter();
    preview.copyMatDepth(image);
    assert(std::abs(image.at<float>(1, 2) - 3.f) < 1e-6);
    preview.copyMatRGB(image);
    std::set<std::array<uint8_t, 3>> distinct;
    for (int y = 0; y < image.rows; ++y)
        for (int x = 0; x < image.cols; ++x)
        {
            const auto c = image.at<Vec3b>(y, x);
            distinct.insert({c[0], c[1], c[2]});
        }
    assert(distinct.size() == 5); // Invalid + four useful depth colors.

    camera.updatePCL();
    assert(cloud.getLastFrame(&positions, &rgb, timestamp) == 4);
    assert(std::abs(positions.front().z() - .5f) < 1e-6);
    assert(std::abs(positions.back().z() - 3.f) < 1e-6);
    assert(std::abs(rgb.front().x() - 90.f / 255.f) < 1e-6);
    camera.updatePCL();
    assert(conversions == 1); // Consume each frame only once.

    frameStatus = SC_INVALID_PARAMS;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(conversions == 1);
    frameStatus = SC_OK;
    depthBytes = 2;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(conversions == 1); // Reject truncated frames before reading them.
    depthBytes = sizeof(depth);
    assert(camera.updateScRGBD());
    depthReady = false;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(conversions == 1); // Do not reuse SDK data after the next capture.
    depthReady = true;

    assert(camera.updateScRGBD());
    convertStatus = SC_INVALID_PARAMS;
    camera.updatePCL();
    assert(conversions == 2 && !camera.m_bPCLframe);
    uint64_t unchanged = 0;
    assert(cloud.getLastFrame(&positions, &rgb, unchanged) == 4 && unchanged == timestamp);
    convertStatus = SC_OK;

    colorWidth = 1;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(cloud.getLastFrame(&positions, &rgb, timestamp) == 4);
    for (const auto &c : rgb) assert(c == Vector3f::Ones());
    colorWidth = 3;
    colorBytes = 1;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(cloud.getLastFrame(&positions, &rgb, timestamp) == 4);
    for (const auto &c : rgb) assert(c == Vector3f::Ones());
    colorBytes = sizeof(colors);

    colorFormat = SC_PIXEL_FORMAT_RGB_888;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(cloud.getLastFrame(&positions, &rgb, timestamp) == 4);
    assert(std::abs(rgb.front().x() - 70.f / 255.f) < 1e-6);
    camera.m_bPCLrgb = false;
    const int before = conversions;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(conversions == before); // Respect disabled point-cloud output.
    camera.m_bPCLrgb = true;

    holdConversion = true;
    auto worker = std::async(std::launch::async, [&] { camera.updatePCL(); });
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    while (!converting && std::chrono::steady_clock::now() < deadline) std::this_thread::yield();
    assert(converting);
    const int priorCaptures = captures;
    std::atomic<bool> captureStarted{false};
    auto capture = std::async(std::launch::async, [&] { captureStarted = true; return camera.updateScRGBD(); });
    while (!captureStarted) std::this_thread::yield();
    assert(capture.wait_for(30ms) == std::future_status::timeout);
    assert(captures == priorCaptures); // SDK buffers cannot be replaced mid-conversion.
    holdConversion = false;
    worker.get();
    assert(capture.get());

    readyStatus = SC_GET_FRAME_READY_TIME_OUT;
    assert(!camera.updateScRGBD());
    const int priorConversions = conversions;
    camera.updatePCL();
    assert(conversions == priorConversions);

    readyStatus = SC_OK;
    colorFormat = SC_PIXEL_FORMAT_BGR_888_JPEG;
    frameHeight = 3; // Odd dimensions exercise the last row and column.
    camera.m_pclStride = 2;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(cloud.getLastFrame(&positions, &rgb, timestamp) == 3);
    // Samples (0,0), (2,0), (0,2), (2,2); (0,0) has invalid depth.
    const std::array<size_t, 3> selected = {2, 6, 8};
    for (size_t i = 0; i < selected.size(); ++i)
    {
        const auto k = selected[i];
        assert(std::abs(positions[i].x() - k * .001f) < 1e-6);
        assert(std::abs(positions[i].z() - depth[k] * .001f) < 1e-6);
        assert(std::abs(rgb[i].x() - colors[k * 3 + 2] / 255.f) < 1e-6);
        assert(std::abs(rgb[i].y() - colors[k * 3 + 1] / 255.f) < 1e-6);
        assert(std::abs(rgb[i].z() - colors[k * 3] / 255.f) < 1e-6);
    }
    camera.copyMatDepth(image);
    assert(image.rows == 3 && image.cols == 3 && image.at<uint16_t>(1, 1) == 2000);
    camera.copyMatTransformedRGB(image);
    assert(image.rows == 3 && image.cols == 3); // Image previews stay full resolution.

    camera.m_bPCLrgb = false;
    camera.m_bPCL = true;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(cloud.getLastFrame(&positions, &rgb, timestamp) == 3);
    for (const auto &c : rgb) assert(c == Vector3f::Ones());

    camera.m_pclStride = 65535; // Larger than the frame: only (0,0), no overrun.
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(cloud.getLastFrame(&positions, &rgb, timestamp) == 0);
    depth[0] = 250;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(cloud.getLastFrame(&positions, &rgb, timestamp) == 1);
    assert(std::abs(positions[0].z() - .25f) < 1e-6);
    depth[0] = 0;

    camera.m_pclStride = 1;
    assert(camera.updateScRGBD());
    camera.updatePCL();
    assert(cloud.getLastFrame(&positions, &rgb, timestamp) == 7); // Full density restored.
    std::cout << "PASS: Scepter invalid depth, D2RGB gradient/meters, PCL errors/colors/concurrency, sampling/config/odd dimensions" << std::endl;
}
