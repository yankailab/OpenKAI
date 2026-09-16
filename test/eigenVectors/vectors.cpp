#include "../../src/Module/JsonCfg.h"
#include "../../src/Universe/Geometry/_GeometryBase.h"
#include "../../src/Universe/Geometry/_GeometryViewerBase.h"

#include <cassert>

template <typename Vector>
void configVector()
{
    using Scalar = typename Vector::Scalar;
    Vector v = Vector::Constant(9);
    const json config = {
        {"short", {1}}, {"empty", json::array()},
        {"long", {2, 3, 4, 5, 6}}, {"invalid", {1, "bad"}}
    };

    assert(jKv<Scalar>(config, "short", v));
    assert(v[0] == 1);
    for (Eigen::Index i = 1; i < v.size(); ++i)
        assert(v[i] == 9);

    const Vector before = v;
    assert(jKv<Scalar>(config, "empty", v));
    assert(v == before);
    assert(!jKv<Scalar>(config, "missing", v));
    assert(v == before);
    assert(!jKv<Scalar>(config, "invalid", v));
    assert(v == before);

    assert(jKv<Scalar>(config, "long", v));
    for (Eigen::Index i = 0; i < v.size(); ++i)
        assert(v[i] == i + 2);
}

int main()
{
    configVector<Vector2f>();
    configVector<Vector3f>();
    configVector<Vector4f>();
    configVector<Vector2d>();
    configVector<Vector3d>();
    configVector<Vector4d>();
    configVector<Vector2i>();
    configVector<Vector3i>();
    configVector<Vector4i>();

    // Existing scalar and std::vector config overloads still work.
    const json config = {{"scalar", 7}, {"array", {1, 2, 3}}};
    int scalar = 0;
    std::vector<int> values;
    assert(jKv(config, "scalar", scalar) && scalar == 7);
    assert(jKv<int>(config, "array", values));
    assert((values == std::vector<int>{1, 2, 3}));

    kai::GEOMETRY_POINT point;
    kai::GEOMETRY_LINE line;
    assert(point.m_vP.isZero());
    assert(line.m_vPa.isZero() && line.m_vPb.isZero());
    assert(point.m_vC == Vector4f(0, 0, 0, 1));
    assert(line.m_vC == Vector4f(0, 0, 0, 1));
    point.m_vP.setOnes();
    line.m_vPa.setOnes();
    line.m_vPb.setOnes();
    point.clear();
    line.clear();
    assert(point.m_vP.isZero() && point.m_vC.isZero() && point.m_tStamp == 0);
    assert(line.m_vPa.isZero() && line.m_vPb.isZero());
    assert(line.m_vC.isZero() && line.m_tStamp == 0);

    kai::GVIEWER_CAM_POSE camera;
    assert(camera.m_vLookAt.isZero());
    assert(camera.m_vEye == Vector3f(0, 0, 1));
    assert(camera.m_vUp == Vector3f(0, 1, 0));

    // Bounding boxes retain left/top/right/bottom component ordering.
    Vector4f box(2, 4, 6, 8);
    assert(kai::bbExpand(box, 2) == Vector4f(0, 2, 8, 10));
    assert(kai::bbExpand(box, 0.5f) == Vector4f(3, 5, 5, 7));
    assert(kai::bbScale(box, 2, 3) == Vector4f(4, 12, 12, 24));
    Vector4f touching(6, 8, 10, 12), separate(7, 9, 10, 12);
    assert(kai::bOverlap(box, touching));
    assert(!kai::bOverlap(box, separate));
    Vector4i pixels(2, 4, 6, 8);
    assert(kai::bbScale(pixels, 0.75f, 0.75f) == Vector4i(1, 3, 4, 6));
}
