#include "../../src/Vision/RGBD/_RGBDbase.h"
#include <cassert>
#include <cmath>
#include <iostream>

namespace
{
    using namespace kai;

    class RGBDTransformHarness : public _RGBDbase
    {
    public:
        void transform(_TransformRigid *value) { m_pTf = value; }
        using _RGBDbase::transformPCL;
    };

    class SnapshotTransform : public _TransformRigid
    {
    public:
        Isometry3f getPoseCombinedf(void) override
        {
            Isometry3f pose = Isometry3f::Identity();
            pose.translation().x() = ++reads;
            return pose;
        }

        unsigned reads = 0;
    };

    class DenseTransform : public _TransformRigid
    {
    public:
        Isometry3f getPoseCombinedf(void) override { ++reads; return pose; }
        Isometry3f pose = Isometry3f::Identity();
        unsigned reads = 0;
    };
}

void testRGBDTransform(void)
{
    using namespace kai;
    JsonCfg cfg;
    cfg.setJson({
        {"parent", {{"class", "_TransformRigid"}, {"vT", {10, 0, 0}},
            {"vR", {0, 0, std::sqrt(0.5), std::sqrt(0.5)}}}},
        {"mount", {{"class", "_TransformRigid"}, {"vT", {1, 0, 0}},
            {"_TransformRigidParent", "parent"}}},
        {"cloud", {{"class", "PCLframe"}}},
        {"camera", {{"class", "_RGBDbase"}, {"PCLframeOut", "cloud"}}}
    });
    json &j = *cfg.getJson();
    InstanceMgr instances;
    assert(instances.addModule("parent", &cfg, &j["parent"]));
    assert(instances.addModule("mount", &cfg, &j["mount"]));
    assert(instances.addDataObject("cloud", &cfg, &j["cloud"]));
    assert(instances.initAll());
    assert(instances.linkAll());

    RGBDTransformHarness camera;
    camera.setName("camera");
    camera.setConfig(&cfg, &j["camera"]);
    assert(camera.loadConfig());
    assert(camera.link(&instances));
    const vector<GEOMETRY_POINT> original = {
        {Vector3f(2, 0, 3), Vector3f(0.1f, 0.2f, 0.3f), 123},
        {Vector3f(0, 1, 2), Vector3f(0.4f, 0.5f, 0.6f), 124}
    };
    auto points = original;
    camera.transformPCL(points);
    for (size_t i = 0; i < points.size(); ++i)
        assert(points[i].m_vP == original[i].m_vP);

    j["camera"]["_TransformRigid"] = "mount";
    assert(camera.link(&instances));
    camera.transformPCL(points);
    assert(points[0].m_vP.isApprox(Vector3f(10, 3, 3)));
    assert(points[1].m_vP.isApprox(Vector3f(9, 1, 2)));
    for (size_t i = 0; i < points.size(); ++i)
    {
        assert(points[i].m_vC == original[i].m_vC);
        assert(points[i].m_tStamp == original[i].m_tStamp);
    }

    auto *parent = static_cast<_TransformRigid *>(instances.findModule("parent"));
    parent->setTranslation(20, 0, 0);
    points = original;
    camera.transformPCL(points);
    assert(points[0].m_vP.isApprox(Vector3f(20, 3, 3)));
    assert(points[1].m_vP.isApprox(Vector3f(19, 1, 2)));

    // Invalid references fail during linking, while an omitted transform stays optional.
    j["camera"]["_TransformRigid"] = "missing";
    assert(!camera.link(&instances));
    j["camera"]["_TransformRigid"] = "cloud";
    assert(!camera.link(&instances));
    j["camera"].erase("_TransformRigid");
    assert(camera.link(&instances));
    points = original;
    camera.transformPCL(points);
    for (size_t i = 0; i < points.size(); ++i)
        assert(points[i].m_vP == original[i].m_vP);

    // An updating transform must be sampled once for the complete frame.
    SnapshotTransform changing;
    camera.transform(&changing);
    camera.transformPCL(points);
    assert(changing.reads == 1);
    for (size_t i = 0; i < points.size(); ++i)
        assert(points[i].m_vP == original[i].m_vP + Vector3f(1, 0, 0));

    // Exercise every matrix coefficient on a complete native-resolution cloud.
    // Only positions may change, and the pose must still be read just once.
    DenseTransform denseTransform;
    denseTransform.pose = Translation3f(1.25f, -.5f, 2.75f) *
        AngleAxisf(.35f, Vector3f::UnitX()) * AngleAxisf(-.55f, Vector3f::UnitY()) *
        AngleAxisf(.75f, Vector3f::UnitZ());
    camera.transform(&denseTransform);
    vector<GEOMETRY_POINT> dense;
    dense.reserve(640 * 480);
    for (int y = 0; y < 480; ++y)
        for (int x = 0; x < 640; ++x)
            dense.push_back({{(x - 320) * .003f, (y - 240) * .003f, 1.f + y * .002f},
                {x / 640.f, y / 480.f, .5f}, uint64_t(123 + y * 640 + x)});
    const auto denseOriginal = dense;
    const auto start = getTns();
    camera.transformPCL(dense);
    const double elapsedMs = double(getTns() - start) / NSEC_MSEC;
    assert(denseTransform.reads == 1);
    for (size_t i = 0; i < dense.size(); ++i)
    {
        const Vector3f expected = denseTransform.pose * denseOriginal[i].m_vP;
        assert(dense[i].m_vP.isApprox(expected, 1e-6f));
        assert(dense[i].m_vC == denseOriginal[i].m_vC);
        assert(dense[i].m_tStamp == denseOriginal[i].m_tStamp);
    }
    std::cout << "Dense RGBD transform: " << dense.size() << " points in " << elapsedMs << " ms\n";
}
