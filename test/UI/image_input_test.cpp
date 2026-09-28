#include "../../src/UI/_UIbase.h"
#include "../../src/UI/_WindowCV.h"
#include "../../src/UI/_GstOutput.h"
#include "../../src/UI/_Console.h"
#include <cassert>
#include <cmath>
#include <limits>

using namespace kai;

namespace
{
    std::map<string, DataStreamBase *> registry;

    class ImageUI : public _UIbase
    {
    public:
        using _UIbase::prepareImage;
    };
}

namespace kai
{
    void _Console::addMsg(const string &, int)
    {
    }

    void _Console::addMsg(const string &, int, int, int)
    {
    }

    InstanceMgr::InstanceMgr() = default;
    InstanceMgr::~InstanceMgr() = default;

    void *InstanceMgr::findDataStream(const string &name)
    {
        auto it = registry.find(name);
        return it == registry.end() ? nullptr : it->second;
    }
}

void checkBlack(const Mat &image, const cv::Size &size)
{
    assert(image.type() == CV_8UC3);
    assert(image.size() == size);
    assert(cv::countNonZero(image.reshape(1)) == 0);
}

void testPreparation()
{
    RGBframe input;
    Mat bgr(2, 3, CV_8UC3, Scalar(10, 20, 30));
    input.set(bgr, 100);
    const auto retainedBGR = input.get();
    Mat image = ImageUI::prepareImage(retainedBGR->m_mRGB, {3, 2});
    assert(image.type() == CV_8UC3);
    assert(image.data == retainedBGR->m_mRGB.data);
    assert(image.at<Vec3b>(0, 0) == Vec3b(10, 20, 30));

    Mat gray(2, 3, CV_8UC1, Scalar(55));
    input.set(gray, 200);
    const auto retainedGray = input.get();
    image = ImageUI::prepareImage(retainedGray->m_mRGB, {6, 4});
    assert(image.size() == cv::Size(6, 4));
    assert(image.at<Vec3b>(0, 0) == Vec3b(55, 55, 55));
    assert(retainedGray->m_mRGB.type() == CV_8UC1);
    assert(cv::norm(retainedGray->m_mRGB, gray, cv::NORM_INF) == 0);

    Mat bgra(2, 3, CV_8UC4, Scalar(11, 22, 33, 44));
    input.set(bgra, 300);
    const auto retainedBGRA = input.get();
    image = ImageUI::prepareImage(retainedBGRA->m_mRGB, {3, 2});
    assert(image.at<Vec3b>(0, 0) == Vec3b(11, 22, 33));
    assert(retainedBGRA->m_mRGB.at<Vec4b>(0, 0) == Vec4b(11, 22, 33, 44));

    Mat depth(1, 2, CV_16UC1);
    depth.at<uint16_t>(0, 0) = 0;
    depth.at<uint16_t>(0, 1) = 1000;
    input.set(depth, 400);
    const auto retainedDepth = input.get();
    image = ImageUI::prepareImage(retainedDepth->m_mRGB, {2, 1});
    assert(image.at<Vec3b>(0, 0) == Vec3b(0, 0, 0));
    assert(image.at<Vec3b>(0, 1) == Vec3b(255, 255, 255));
    assert(retainedDepth->m_mRGB.at<uint16_t>(0, 1) == 1000);

    Mat thermal(1, 3, CV_32FC1);
    thermal.at<float>(0, 0) = 0;
    thermal.at<float>(0, 1) = 1;
    thermal.at<float>(0, 2) = std::numeric_limits<float>::quiet_NaN();
    input.set(thermal, 500);
    const auto retainedThermal = input.get();
    image = ImageUI::prepareImage(retainedThermal->m_mRGB, {3, 1});
    assert(image.at<Vec3b>(0, 0) == Vec3b(0, 0, 0));
    assert(image.at<Vec3b>(0, 1) == Vec3b(255, 255, 255));
    assert(image.at<Vec3b>(0, 2) == Vec3b(0, 0, 0));
    assert(std::isnan(retainedThermal->m_mRGB.at<float>(0, 2)));

    checkBlack(ImageUI::prepareImage(Mat(2, 2, CV_8UC2), {5, 4}), {5, 4});
    input.set(Mat(), 600);
    const auto empty = input.get();
    checkBlack(ImageUI::prepareImage(empty->m_mRGB, {5, 4}), {5, 4});
    assert(ImageUI::prepareImage(empty->m_mRGB, {0, 4}).empty());
    assert(ImageUI::prepareImage(empty->m_mRGB, {5, -1}).empty());
    assert(retainedBGR->m_mRGB.at<Vec3b>(0, 0) == Vec3b(10, 20, 30));
}

template <typename T>
void testLink()
{
    RGBframe input;
    DataStreamBase wrongType;
    registry = {{"input", &input}, {"wrong", &wrongType}};
    InstanceMgr manager;
    JsonCfg cfg;
    json j = {{"RGBframeIn", "input"}};
    T ui;
    ui.setConfig(&cfg, &j);
    assert(ui.link(&manager));
    j["RGBframeIn"] = "wrong";
    assert(!ui.link(&manager));
    j["RGBframeIn"] = "missing";
    assert(!ui.link(&manager));
    j.erase("RGBframeIn");
    assert(!ui.link(&manager));
}

int main()
{
    testPreparation();
    testLink<_WindowCV>();
    testLink<_GstOutput>();
    std::cout << "PASS: UI stream links, BGR/gray/BGRA conversion, resizing, depth/thermal display, immutable input, empty-frame clearing\n";
}
