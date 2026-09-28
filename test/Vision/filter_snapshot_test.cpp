#include "../../src/Base/_ModuleBase.h"
#include "../../src/Protocol/_JSONbase.h"
#include "../../src/DataStream/RGBframe.h"
#include "../../src/DataStream/RGBDframe.h"
#include "../../src/DataStream/PCLframe.h"
#include "../../src/DataStream/IMUstream.h"
#include "../../src/UI/_Console.h"
#include <cassert>
// Expose filter entry points only in this standalone test translation unit.
#define private public
#define protected public
#include "../../src/Vision/Pipeline/_ColorConvert.h"
#include "../../src/Vision/Pipeline/_Contrast.h"
#include "../../src/Vision/Pipeline/_Crop.h"
#include "../../src/Vision/Pipeline/_D2RGB.h"
#include "../../src/Vision/Pipeline/_Erode.h"
#include "../../src/Vision/Pipeline/_HistEqualize.h"
#include "../../src/Vision/Pipeline/_InRange.h"
#include "../../src/Vision/Pipeline/_Invert.h"
#include "../../src/Vision/Pipeline/_Mask.h"
#include "../../src/Vision/Pipeline/_Morphology.h"
#include "../../src/Vision/Pipeline/_Remap.h"
#include "../../src/Vision/Pipeline/_Resize.h"
#include "../../src/Vision/Pipeline/_Rotate.h"
#include "../../src/Vision/Pipeline/_Thermal2RGB.h"
#include "../../src/Vision/Pipeline/_Threshold.h"
#undef private
#undef protected

using namespace kai;
// Isolate the stream links from application/UI startup. The actual BASE,
// _ModuleBase, _Thread, and JsonCfg implementations are linked.
std::map<string, DataStreamBase *> registry;
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

void same(const Mat &a, const Mat &b)
{
    assert(a.size() == b.size() && a.type() == b.type());
    assert(cv::norm(a, b, cv::NORM_INF) < 1e-6);
}

template <class T>
void run(T &filter, RGBframe &input, RGBframe &output)
{
    filter.m_pRGBin = &input;
    filter.m_pRGB = &output;
    const auto retained = input.get();
    const Mat original = retained->m_mRGB.clone();
    filter.filter();
    assert(output.get()->m_tStamp == retained->m_tStamp);
    same(retained->m_mRGB, original);
}

void testFilters()
{
    RGBframe input;
    RGBframe output;
    Mat source(4, 6, CV_8UC3, Scalar(10, 40, 100));
    input.set(source, 123);
    Mat result;
    Mat expected;
    _ColorConvert convert;
    convert.m_code = COLOR_BGR2GRAY;
    run(convert, input, output);
    result = output.get()->m_mRGB;
    assert(result.type() == CV_8UC1 && result.at<uint8_t>(0, 0) == 55);
    _Contrast contrast;
    contrast.m_alpha = 2;
    contrast.m_beta = 5;
    run(contrast, input, output);
    result = output.get()->m_mRGB;
    same(result, Mat(4, 6, CV_8UC3, Scalar(25, 85, 205)));
    _Invert invert;
    run(invert, input, output);
    result = output.get()->m_mRGB;
    same(result, Mat(4, 6, CV_8UC3, Scalar(245, 215, 155)));
    _Crop crop;
    crop.m_vRoi = {1, 1, 4, 3};
    run(crop, input, output);
    result = output.get()->m_mRGB;
    same(result, source(Rect(1, 1, 3, 2)));
    _Resize resize;
    resize.m_vSizeRGB = {3, 2};
    run(resize, input, output);
    result = output.get()->m_mRGB;
    same(result, Mat(2, 3, CV_8UC3, Scalar(10, 40, 100)));
    _Rotate rotate;
    rotate.m_code = ROTATE_90_CLOCKWISE;
    run(rotate, input, output);
    result = output.get()->m_mRGB;
    same(result, Mat(6, 4, CV_8UC3, Scalar(10, 40, 100)));
    _InRange inRange;
    inRange.m_vL = {0, 0, 0};
    inRange.m_vH = {20, 60, 120};
    run(inRange, input, output);
    result = output.get()->m_mRGB;
    same(result, Mat(4, 6, CV_8UC1, Scalar(255)));
    _Erode erode;
    run(erode, input, output);
    result = output.get()->m_mRGB;
    same(result, source);
    _Morphology morph;
    run(morph, input, output);
    result = output.get()->m_mRGB;
    same(result, source);
    _Threshold threshold;
    Mat gray(5, 5, CV_8UC1, Scalar(70));
    input.set(gray);
    IMG_THRESHOLD rule;
    rule.init();
    rule.m_type = img_thr;
    rule.m_thr = 50;
    rule.m_vMax = 255;
    threshold.m_vFilter.push_back(rule);
    run(threshold, input, output);
    result = output.get()->m_mRGB;
    same(result, Mat(5, 5, CV_8UC1, Scalar(255)));
    input.set(source);
    _HistEqualize hist;
    run(hist, input, output);
    result = output.get()->m_mRGB;
    assert(!result.empty() && result.type() == CV_8UC3);
    _Remap remap;
    run(remap, input, output);
    result = output.get()->m_mRGB;
    same(result, source);
    RGBframe mask;
    Mat maskPixels = Mat::zeros(source.size(), CV_8UC1);
    maskPixels.at<uint8_t>(1, 2) = 255;
    mask.set(maskPixels, 987654);
    const auto retainedMask = mask.get();
    _Mask applyMask;
    applyMask.m_pMask = &mask;
    run(applyMask, input, output);
    same(retainedMask->m_mRGB, maskPixels);
    result = output.get()->m_mRGB;
    expected = Mat::zeros(source.size(), source.type());
    expected.at<Vec3b>(1, 2) = source.at<Vec3b>(1, 2);
    same(result, expected);
    _Thermal2RGB thermal;
    input.set(Mat(2, 2, CV_32FC1, Scalar(20)));
    run(thermal, input, output);
    result = output.get()->m_mRGB;
    assert(result.type() == CV_8UC3 && result.rows == 2);
    // Missing output is harmless; in-place stream linking remains valid.
    thermal.m_pRGB = nullptr;
    thermal.filter();
    input.set(source);
    run(invert, input, input);
    result = input.get()->m_mRGB;
    same(result, Mat(4, 6, CV_8UC3, Scalar(245, 215, 155)));
    _D2RGB depth;
    RGBframe distances;
    RGBDframe pair;
    input.set(Mat(4, 6, CV_32FC1, Scalar(2.5)), 76543);
    const auto retainedDepth = input.get();
    depth.m_pDin = &input;
    depth.m_pRGB = &output;
    depth.m_pD = &distances;
    depth.m_pRGBD = &pair;
    depth.m_vRangeD = {2, 4};
    depth.m_nHistLev = 4;
    depth.filter();
    const auto depthFrame = pair.get();
    result = depthFrame->m_mRGB;
    expected = depthFrame->m_mD;
    assert(depthFrame->m_tStamp == retainedDepth->m_tStamp);
    assert(output.get()->m_tStamp == retainedDepth->m_tStamp);
    assert(distances.get()->m_tStamp == retainedDepth->m_tStamp);
    same(retainedDepth->m_mRGB, Mat(4, 6, CV_32FC1, Scalar(2.5)));
    assert(result.type() == CV_8UC3 && expected.type() == CV_32FC1);
    assert(depth.d({0, 0, 6, 4}) == 2.5f);
    assert(depth.d({30, 30, 40, 40}) == -1);
    input.set(Mat());
    depth.filter();
    assert(depth.d({0, 0, 6, 4}) == -1);
    std::cout << "PASS: all 15 filters, masking, depth measurements, empty input and in-place streams\n";
}

void testLinks()
{
    RGBframe input;
    RGBframe output;
    RGBDframe wrongType;
    registry = {{"input", &input}, {"output", &output}, {"wrong", &wrongType}};
    InstanceMgr manager;
    JsonCfg cfg;
    json j = {{"RGBframeIn", "input"}, {"RGBframe", "output"}};
    _Invert filter;
    filter.setConfig(&cfg, &j);
    assert(filter.link(&manager));
    assert(filter.m_pRGBin == &input && filter.m_pRGB == &output);

    j["RGBframeIn"] = "wrong";
    assert(!filter.link(&manager));
    j["RGBframeIn"] = "missing";
    assert(!filter.link(&manager));
    j["RGBframeIn"] = "input";
    j["RGBframe"] = "wrong";
    assert(!filter.link(&manager));
    j["RGBframe"] = "missing";
    assert(!filter.link(&manager));
    j.erase("RGBframe");
    assert(filter.link(&manager));
    std::cout << "PASS: direct stream links, missing names, wrong stream types, optional outputs\n";
}

void testMultiStageSnapshots()
{
    RGBframe input;
    RGBframe output;
    Mat source(9, 9, CV_8UC1);
    cv::randu(source, 0, 255);
    input.set(source, 12345);
    _Erode erode;
    IMG_ERODE erosion;
    erosion.init();
    erosion.updateKernel();
    erode.m_vFilter = {erosion, erosion, erosion};
    run(erode, input, output);
    Mat expected;
    cv::erode(source, expected, erosion.m_kernel, {-1, -1}, 3);
    same(output.get()->m_mRGB, expected);

    _Morphology morphology;
    IMG_MORPH morph;
    morph.init();
    morph.m_morphOp = MORPH_DILATE;
    morph.updateKernel();
    morphology.m_vFilter = {morph, morph, morph};
    run(morphology, input, output);
    cv::dilate(source, expected, morph.m_kernel, {-1, -1}, 3);
    same(output.get()->m_mRGB, expected);

    _Threshold threshold;
    IMG_THRESHOLD rule;
    rule.init();
    rule.m_type = img_thr;
    rule.m_thr = 128;
    rule.m_vMax = 255;
    threshold.m_vFilter = {rule, rule, rule};
    run(threshold, input, output);
    cv::threshold(source, expected, 128, 255, THRESH_BINARY);
    same(output.get()->m_mRGB, expected);

    Mat color(9, 9, CV_8UC3);
    cv::randu(color, 0, 255);
    input.set(color, 34567);
    run(threshold, input, output);
    cv::cvtColor(color, expected, COLOR_RGB2GRAY);
    cv::threshold(expected, expected, 128, 255, THRESH_BINARY);
    same(output.get()->m_mRGB, expected);
    std::cout << "PASS: multi-stage filters preserve retained inputs and capture timestamps\n";
}

int main()
{
    cv::setRNGSeed(1234);
    testFilters();
    testLinks();
    testMultiStageSnapshots();
}
