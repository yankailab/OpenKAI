#include "../../src/Universe/Grid/_OctreeGridBase.h"
#include <cassert>
#include <limits>

using namespace kai;

namespace
{
// A grid with no PCL dependency, occupancy counts, colors or timestamps.
class ScalarGrid : public _OctreeGridBase
{
public:
    using _OctreeGridBase::visitCells;
    OCTGRID_CELL<double> root;
    ~ScalarGrid() override { root.release(); }

    OCTGRID_CELL<double> *addCell(const Vector3f &position, int depth = -1)
    {
        return visitCells(&root, position, depth, true, [](OCTGRID_CELL<double> *) { return true; });
    }
};

UUID128 pathID(const vector<unsigned> &path)
{
    UUID128 id(0, uint64_t(path.size()) << 56);
    for (size_t level = 0; level < path.size(); ++level)
        for (unsigned bit = 0; bit < 3; ++bit)
            if (path[level] & (1u << bit))
            {
                const unsigned offset = 117 - 3 * level + bit;
                id.m_uint64[offset / 64] |= uint64_t(1) << (offset % 64);
            }
    return id;
}
}

void testOctreeGridBase()
{
    JsonCfg config;
    config.setJson({{"grid", {{"class", "_OctreeGridBase"}}}});
    ScalarGrid grid;
    json &settings = config.getJson()->at("grid");
    grid.setName("grid");
    grid.setConfig(&config, &settings);
    assert(grid.loadConfig() && grid.saveConfig(false));
    assert(settings.at("vPorigin") == json({0, 0, 0}));
    assert(settings.at("vRootCellSize") == json({10, 10, 10}));
    assert(settings.at("nMaxLevel") == 4);
    assert(!settings.contains("vPCLframesIn") && !settings.contains("dTexpireCell"));

    // Captured visitors and typed child creation work across translation units.
    const double sample = 2.5;
    unsigned visits = 0;
    auto *visited = grid.visitCells(&grid.root, Vector3f(4, 4, 4), 2, true,
        [&](OCTGRID_CELL<double> *cell)
        {
            *cell->addT() += sample;
            ++visits;
            return true;
        });
    assert(visited && visited->getLevel() == 2 && visits == 3);
    assert(grid.getCell(&grid.root, pathID({7, 7})) == visited);
    assert(*grid.root.getT() == sample && *grid.root.getChild(7)->getT() == sample);
    visits = 0;
    assert(!grid.visitCells(&grid.root, Vector3f(-4, -4, -4), 3, true,
        [&](OCTGRID_CELL<double> *cell) { ++visits; return cell->getLevel() < 1; }));
    assert(visits == 2 && grid.root.getChild(0) && !grid.root.getChild(0)->getChild(0));
    assert(!grid.visitCells(&grid.root, Vector3f(6, 0, 0), 3, true,
        [&](OCTGRID_CELL<double> *) { ++visits; return true; }));
    assert(visits == 2); // An invalid point must never invoke the visitor.
    grid.root.release();

    settings["vPorigin"] = {1, -2, 3};
    settings["vRootCellSize"] = {8, 16, 32};
    settings["nMaxLevel"] = 40;
    assert(grid.loadConfig() && grid.saveConfig(false));
    ScalarGrid restored;
    restored.setConfig(&config, &settings);
    assert(restored.loadConfig());
    const Vector3f corner(5, 6, 19);
    auto *leaf = restored.addCell(corner);
    assert(leaf && leaf->getLevel() == 40 && !leaf->getT());
    assert(restored.getCell(&restored.root, leaf->m_ID) == leaf);
    assert(restored.getCell(&restored.root, corner) == leaf);
    assert(!leaf->addChild(0)); // IDs have exactly forty path segments.
    assert(!restored.root.addChild(N_OCT));
    for (unsigned depth = 0; depth <= 40; ++depth)
    {
        auto *cell = restored.getCell(&restored.root, pathID(vector<unsigned>(depth, 7)));
        assert(cell && cell->getLevel() == int(depth) && !cell->getT());
        assert(restored.getCell(&restored.root, corner, depth) == cell);
    }
    *leaf->addT() = 12.5;
    const UUID128 leafID = leaf->m_ID;
    assert(*restored.getCell(&restored.root, leafID)->getT() == 12.5);
    leaf->release();
    assert(leaf->getLevel() == 40 && leaf->m_pParent && !leaf->getT());
    assert(restored.getCell(&restored.root, leafID) == leaf);

    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    for (const auto &position : {Vector3f(nan, 0, 0), Vector3f(0, inf, 0), Vector3f(5.01f, 6, 19)})
    {
        assert(!restored.addCell(position));
        assert(!restored.getCell(&restored.root, position));
    }
    assert(!restored.getCell(&restored.root, UUID128(1)));
    assert(!restored.getCell(&restored.root, UUID128(0, uint64_t(1) << 63)));
    restored.root.deleteChild(7);
    assert(!restored.getCell(&restored.root, leafID));

    // IDs and lookup also work for payloads with nontrivial lifetime, at word boundaries.
    OCTGRID_CELL<string> textRoot;
    auto *textCell = &textRoot;
    vector<unsigned> path;
    for (unsigned depth = 0; depth < 40; ++depth)
    {
        const unsigned child = (depth * 5 + 3) % N_OCT;
        path.push_back(child);
        textCell = textCell->addChild(child);
        assert(textCell && restored.getCell(&textRoot, pathID(path)) == textCell);
        assert(octgridCellChild(textCell->m_ID, depth) == child);
    }
    *textCell->addT() = "label";
    assert(*restored.getCell(&textRoot, pathID(path))->getT() == "label");
    textRoot.release();
    assert(!restored.getCell(&textRoot, pathID(path)));
    assert(restored.getCell(&textRoot, UUID128(0)) == &textRoot);

    for (const int depth : {-1, 41})
    {
        settings["nMaxLevel"] = depth;
        assert(!grid.loadConfig());
    }
    settings["nMaxLevel"] = 4;
    for (const float size : {0.f, -1.f, inf, nan})
    {
        settings["vRootCellSize"] = {8, size, 32};
        assert(!grid.loadConfig());
    }
    settings["vRootCellSize"] = {8, 16, 32};
    settings["vPorigin"] = {0, nan, 0};
    assert(!grid.loadConfig());
}
