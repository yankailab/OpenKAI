#include "../../src/Universe/Grid/_SelectableOctGrid.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace kai;

namespace
{
class GridHarness : public _SelectableOctGrid
{
public:
    void attach(PCLframe &frame) { m_vPointInputs.push_back({&frame, 0}); }
    void tick() { updateGrid(); }

    void ageCells(uint64_t stamp)
    {
        auto age = [stamp](auto &&self, OCTREE_CELL<OCTGRID_PCL_CELL> *cell) -> void
        {
            if (!cell) return;
            if (auto *value = cell->getT()) value->m_tStamp = stamp;
            for (unsigned i = 0; i < N_OCT; ++i) self(self, cell->getChild(i));
        };
        age(age, m_pCell);
    }
};

struct Fixture
{
    JsonCfg config;
    GridHarness grid;
    PCLframe input;

    explicit Fixture(json settings)
    {
        settings["class"] = "_SelectableOctGrid";
        config.setJson({{"grid", std::move(settings)}});
        grid.setName("grid");
        grid.setConfig(&config, &(*config.getJson())["grid"]);
        assert(grid.loadConfig());
        grid.attach(input);
    }

    void publish(vector<GEOMETRY_POINT> points)
    {
        const auto stamp = getTns();
        for (auto &point : points) point.m_tStamp = stamp;
        input.set(points, stamp);
        grid.tick();
    }
};

void testCountsColorsAndPaths()
{
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}, {"nPminBuild", 0}});
    vector<GEOMETRY_POINT> points;
    for (unsigned octant = 0; octant < 8; ++octant)
    {
        const Vector3f position((octant & 4) ? 1.5f : -1.5f,
                                (octant & 2) ? 1.5f : -1.5f,
                                (octant & 1) ? 1.5f : -1.5f);
        points.push_back({position, {1, 0, 0}, 0});
        points.push_back({position, {0, 0, 1}, 0});
    }
    f.publish(points);
    const auto *root = f.grid.getCell(UUID128(0));
    assert(root && root->m_nP == 16);
    assert((root->m_vC - Vector4f(.5f, 0, .5f, 1)).norm() < 1e-6f);
    OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 17); // Root, eight parents, eight leaves.
    for (unsigned octant = 0; octant < 8; ++octant)
    {
        const auto &position = points[octant * 2].m_vP;
        const auto *leaf = f.grid.getCell(position);
        assert(leaf && leaf->m_nP == 2 && f.grid.getCell(leaf->m_ID) == leaf);
        assert(leaf->m_ID.m_uint64[0] == 2);
        assert(leaf->m_ID.m_uint64[1] == (uint64_t(octant) << 59 | uint64_t(octant) << 56));
        assert((leaf->m_vC - Vector4f(.5f, 0, .5f, 1)).norm() < 1e-6f);
        std::array<float, 3> center, size;
        assert(octgridCellBox(snapshot.m_header, leaf->m_ID, center, size));
        for (unsigned axis = 0; axis < 3; ++axis)
            assert(center[axis] == position[axis] && size[axis] == 1);
    }

    f.grid.tick();
    assert(f.grid.getCell(UUID128(0))->m_nP == 16); // No recount of an unchanged frame.
    f.publish({{points.front().m_vP, {0, 1, 0}, 0}});
    const auto *leaf = f.grid.getCell(points.front().m_vP);
    assert(leaf && leaf->m_nP == 3);
    assert((leaf->m_vC - Vector4f(1.f / 3, 1.f / 3, 1.f / 3, 1)).norm() < 1e-6f);
    assert(!f.grid.addCellPoint({{2.01f, 0, 0}, {1, 1, 1}, getTns()}, getTns()));
    assert(f.grid.getCell(UUID128(0))->m_nP == 17);
}

void testStaleAndInvalidPoints()
{
    Fixture f({{"nMaxLevel", 2}, {"nPminBuild", 0},
               {"dTexpireCell", 300000000}, {"dTexpirePCL", 500000000}});
    const auto now = getTns();
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    f.input.set({{{0, 0, 1}, {1, 0, 0}, now - NSEC_SEC},
                 {{0, 0, 1}, {1, 0, 0}, 0},
                 {{nan, 0, 1}, {1, 0, 0}, now},
                 {{0, inf, 1}, {1, 0, 0}, now},
                 {{0, 0, -inf}, {1, 0, 0}, now},
                 {{6, 0, 1}, {1, 0, 0}, now}}, now);
    f.grid.tick();
    OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 0);
    f.publish({{{0, 0, 1}, {.25f, .5f, .75f}, 0}});
    assert(f.grid.get(&snapshot) == 3);
    f.grid.ageCells(getTns() - NSEC_SEC);
    f.grid.tick();
    assert(f.grid.get(&snapshot) == 0);
    assert(!f.grid.getCell(UUID128(0))); // Expiry remains active with no new input.
}

void testDenseSampleCloud()
{
    // Reproduce the sample's original 300 ms cell expiry. Its unoptimized
    // per-point Eigen arithmetic took over a second and erased the entire
    // freshly constructed grid before publishing any occupied cells.
    Fixture f({{"vRootCellSize", {5, 5, 5}}, {"nMaxLevel", 5},
               {"nMaxCells", 18333}, {"nPminBuild", 100},
               {"dTexpireCell", 300000000}, {"dTexpirePCL", 500000000}});
    vector<GEOMETRY_POINT> points;
    points.reserve(640 * 480);
    for (int y = 0; y < 480; ++y)
        for (int x = 0; x < 640; ++x)
            points.push_back({{(x - 320) * .003f, (y - 240) * .003f, 1}, {.25f, .5f, .75f}, 0});
    const auto start = getTns();
    f.publish(std::move(points));
    const double elapsedMs = double(getTns() - start) / NSEC_MSEC;
    OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 213);
    const auto *root = f.grid.getCell(UUID128(0));
    assert(root && root->m_nP == 640 * 480);
    assert((root->m_vC - Vector4f(.25f, .5f, .75f, 1)).norm() < 1e-6f);
    assert(snapshot.m_header.m_nMaxLevel == 5);
    assert(snapshot.m_header.m_vRootCellSize == (std::array<float, 3>{5, 5, 5}));
    std::cout << "Dense sample cloud: " << snapshot.m_vCell.size()
              << " occupied cells in " << elapsedMs << " ms\n";
}
}

int main()
{
    testCountsColorsAndPaths();
    testStaleAndInvalidPoints();
    testDenseSampleCloud();
    std::cout << "Octree grid regressions passed\n";
}
