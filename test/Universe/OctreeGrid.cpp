#include "../../src/Universe/Grid/_SelectableOctGrid.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace kai;

namespace
{
UUID128 idForPath(const vector<unsigned> &path)
{
    UUID128 id(0, uint64_t(path.size()) << 56);
    // Assign each path bit independently to exercise UUID word-boundary handling.
    for (size_t level = 0; level < path.size(); ++level)
        for (unsigned bit = 0; bit < 3; ++bit)
            if (path[level] & (1u << bit))
            {
                const unsigned offset = 117 - 3 * level + bit;
                id.m_uint64[offset / 64] |= uint64_t(1) << (offset % 64);
            }
    return id;
}

bool sameID(const UUID128 &a, const UUID128 &b)
{
    return a.m_uint64[0] == b.m_uint64[0] && a.m_uint64[1] == b.m_uint64[1];
}

string idHex(const UUID128 &id)
{
    constexpr char digits[] = "0123456789abcdef";
    OCTGRID_CELL cell;
    cell.setID(id);
    string result;
    for (const uint8_t byte : cell.m_ID)
    {
        result += digits[byte >> 4];
        result += digits[byte & 15];
    }
    return result;
}

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
        assert(leaf->m_ID.m_uint64[0] == 0);
        assert(leaf->m_ID.m_uint64[1] == (uint64_t(2) << 56 | uint64_t(octant) << 53 | uint64_t(octant) << 50));
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

void testCellIDLayoutAndGeometry()
{
    assert(sameID(idForPath({5}), UUID128(0, 0x01a0000000000000)));
    assert(idHex(idForPath({5})) == "0000000000000000000000000000a001");
    vector<unsigned> crossingPath(19, 0);
    crossingPath.back() = 7;
    assert(sameID(idForPath(crossingPath), UUID128(0x8000000000000000, 0x1300000000000003)));
    assert(sameID(idForPath(vector<unsigned>(40, 7)), UUID128(UINT64_MAX, 0x28ffffffffffffff)));
    vector<unsigned> lastPath(40, 0);
    lastPath.back() = 5;
    assert(sameID(idForPath(lastPath), UUID128(5, 0x2800000000000000)));

    const OCTGRID_HEADER header = {{1, -2, 3}, {8, 16, 32}, 40, 0};
    vector<unsigned> path;
    std::array<uint64_t, 3> coordinates = {};
    for (unsigned depth = 0; depth <= 40; ++depth)
    {
        const UUID128 id = idForPath(path);
        OCTGRID_CELL record;
        record.setID(id);
        assert(record.m_ID[15] == depth && sameID(record.id(), id));
        std::array<float, 3> center, size;
        assert(octgridCellBox(header, id, center, size));
        for (unsigned axis = 0; axis < 3; ++axis)
        {
            const double subdivisions = std::ldexp(1., depth);
            const float expectedSize = header.m_vRootCellSize[axis] / subdivisions;
            const float expectedCenter = header.m_vPorigin[axis] + header.m_vRootCellSize[axis] *
                ((coordinates[axis] + .5) / subdivisions - .5);
            assert(size[axis] == expectedSize && center[axis] == expectedCenter);
        }
        if (depth < 40)
        {
            const unsigned child = (depth * 5 + 3) % 8;
            path.push_back(child);
            for (unsigned axis = 0; axis < 3; ++axis)
                coordinates[axis] = coordinates[axis] * 2 + bool(child & (4 >> axis));
        }
    }
}

void testDeepGeneratedIDsAndSelection()
{
    const UUID128 selectedID(UINT64_MAX, 0x28ffffffffffffff);
    const string selectedHex = "ffffffffffffffffffffffffffffff28";
    Fixture f({{"vRootCellSize", {2, 2, 2}}, {"nMaxLevel", 40}, {"nPminBuild", 0},
               {"vSelectedCells", {selectedHex}}});
    assert(f.grid.getSelectedCells().size() == 1);
    assert(sameID(f.grid.getSelectedCells().front(), selectedID));
    assert(!f.grid.bSelectedOccupied(0));
    // The positive root corner chooses child 7 at every depth, including where
    // float cell centers round to the input point near the deepest levels.
    const Vector3f position(1, 1, 1);
    f.publish({{position, {.2f, .4f, .6f}, 0}});
    OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 41);
    for (unsigned depth = 0; depth <= 40; ++depth)
    {
        const UUID128 id = idForPath(vector<unsigned>(depth, 7));
        const auto *cell = f.grid.getCell(position, depth);
        assert(cell && sameID(cell->m_ID, id));
        assert(f.grid.getCell(id) == cell);
        assert(sameID(snapshot.m_vCell[depth].id(), id));
    }
    assert(f.grid.bSelectedOccupied(0));
    assert(!f.grid.bSelectedOccupied(1));
    assert(f.grid.saveConfig(false));
    assert((*f.config.getJson())["grid"]["vSelectedCells"] == json::array({selectedHex}));
    assert(f.grid.loadConfig());
    assert(sameID(f.grid.getSelectedCells().front(), selectedID));
}

void testInvalidCellIDs()
{
    Fixture f({{"vRootCellSize", {2, 2, 2}}, {"nMaxLevel", 40}, {"nPminBuild", 0},
               {"vSelectedCells", {"0000000000000000000000000000e001"}}});
    f.publish({{{1, 1, 1}, {1, 1, 1}, 0}});
    OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 41);
    const vector<UUID128> invalid = {
        {0, uint64_t(41) << 56}, {0, uint64_t(63) << 56}, // Unsupported depths.
        {0, uint64_t(1) << 62}, {0, uint64_t(1) << 63}, // Reserved bits.
        {1, 0}, {0, uint64_t(1) << 53}, // Root with nonzero path bits.
        {1, 0x01e0000000000000}, // A valid first child followed by unused path bits.
        {uint64_t(1) << 63, uint64_t(18) << 56}, // Unused bits at the UUID word boundary.
        {uint64_t(1) << 60, uint64_t(19) << 56},
        {1, uint64_t(39) << 56}, // Final path segment must be zero before depth 40.
        {1, uint64_t(7) << 59}, // Former depth-1 child-7 layout is invalid now.
    };
    for (const auto &id : invalid)
    {
        std::array<float, 3> center, size;
        assert(!octgridCellBox(snapshot.m_header, id, center, size));
        assert(!f.grid.getCell(id));
        const UUID128 selected = f.grid.getSelectedCells().front();
        f.grid.jsonCmd({{"cmd", "octGridCellSelect"}, {"vPorigin", {"0", "0", "0"}},
                        {"vRootCellSize", {"2", "2", "2"}}, {"cellIDs", {idHex(id)}}}, nullptr);
        assert(f.grid.getSelectedCells().size() == 1 && sameID(f.grid.getSelectedCells().front(), selected));
        (*f.config.getJson())["grid"]["vSelectedCells"] = json::array({idHex(id)});
        assert(!f.grid.loadConfig());
        (*f.config.getJson())["grid"]["vSelectedCells"] = json::array({idHex(selected)});
        assert(f.grid.loadConfig());
        f.publish({{{1, 1, 1}, {1, 1, 1}, 0}});
    }
    auto limitedHeader = snapshot.m_header;
    limitedHeader.m_nMaxLevel = 18;
    std::array<float, 3> center, size;
    assert(!octgridCellBox(limitedHeader, idForPath(vector<unsigned>(19, 0)), center, size));
    limitedHeader.m_nMaxLevel = 41;
    assert(!octgridCellBox(limitedHeader, UUID128(0), center, size));
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
    testCellIDLayoutAndGeometry();
    testDeepGeneratedIDsAndSelection();
    testInvalidCellIDs();
    testStaleAndInvalidPoints();
    testDenseSampleCloud();
    std::cout << "Octree grid regressions passed\n";
}
