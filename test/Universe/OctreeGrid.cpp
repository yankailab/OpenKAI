#include "../../src/Universe/Grid/_SelectableOctGrid.h"
#include "../../src/Protocol/_JSONbase.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <limits>
#include <unistd.h>

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
    SELECTABLE_OCTGRID_CELL cell;
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
    string configPath;

    explicit Fixture(json settings, bool persist = false)
    {
        settings["class"] = "_SelectableOctGrid";
        config.setJson({{"grid", std::move(settings)}});
        if (persist)
        {
            char path[] = "/tmp/openkai-grid-config-XXXXXX";
            const int fd = mkstemp(path);
            assert(fd >= 0);
            close(fd);
            configPath = path;
            assert(config.saveToFile(configPath) && config.readFromFile(configPath));
        }
        grid.setName("grid");
        grid.setConfig(&config, &(*config.getJson())["grid"]);
        assert(grid.loadConfig());
        grid.attach(input);
    }

    ~Fixture()
    {
        if (!configPath.empty()) std::remove(configPath.c_str());
    }

    void publish(vector<GEOMETRY_POINT> points)
    {
        const auto stamp = getTns();
        for (auto &point : points) point.m_tStamp = stamp;
        input.set(points, stamp);
        grid.tick();
    }
};

class GridReplyTransport : public _JSONbase
{
public:
    vector<json> replies;
    bool sendJson(const json &reply) override
    {
        replies.push_back(reply);
        return true;
    }
};

json gridCommand(GridHarness &grid, json command)
{
    GridReplyTransport transport;
    command["module"] = "grid";
    command["requestId"] = "grid-test";
    grid.jsonCmd(command, &transport);
    assert(transport.replies.size() == 1);
    const json reply = transport.replies.front();
    assert(reply.at("cmd") == command.at("cmd") && reply.at("module") == "grid");
    assert(reply.at("requestId") == "grid-test");
    return reply;
}

json gridConfig(GridHarness &grid)
{
    json reply = gridCommand(grid, {{"cmd", "getGridConfig"}});
    assert(reply.at("bSuccess") == true);
    for (const char *key : {"cmd", "module", "requestId", "bSuccess"}) reply.erase(key);
    return reply;
}

json setGridConfig(GridHarness &grid, json config)
{
    config["cmd"] = "setGridConfig";
    return gridCommand(grid, std::move(config));
}

json cellSelection(GridHarness &grid)
{
    json reply = gridCommand(grid, {{"cmd", "getCellSelection"}});
    assert(reply.at("bSuccess") == true);
    assert(reply.at("tStamp").is_string() && std::stoull(reply.at("tStamp").get<string>()) > 0);
    for (const char *key : {"cmd", "module", "requestId", "bSuccess", "tStamp"}) reply.erase(key);
    return reply;
}

json setCellSelection(GridHarness &grid, json selection)
{
    selection["cmd"] = "setCellSelection";
    return gridCommand(grid, std::move(selection));
}

json storedConfig(const Fixture &f)
{
    JsonCfg saved;
    assert(saved.readFromFile(f.configPath));
    return saved.getJson()->at("grid");
}

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
    SELECTABLE_OCTGRID_CELLS snapshot;
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

    const SELECTABLE_OCTGRID_HEADER header = {{1, -2, 3}, {8, 16, 32}, 40, 0};
    vector<unsigned> path;
    std::array<uint64_t, 3> coordinates = {};
    for (unsigned depth = 0; depth <= 40; ++depth)
    {
        const UUID128 id = idForPath(path);
        SELECTABLE_OCTGRID_CELL record;
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
    SELECTABLE_OCTGRID_CELLS snapshot;
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
    SELECTABLE_OCTGRID_CELLS snapshot;
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
        f.grid.jsonCmd({{"cmd", "setCellSelection"}, {"vPorigin", {"0", "0", "0"}},
                        {"vRootCellSize", {"2", "2", "2"}}, {"nMaxLevel", 40}, {"cellIDs", {idHex(id)}}}, nullptr);
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
    SELECTABLE_OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 0);
    f.publish({{{0, 0, 1}, {.25f, .5f, .75f}, 0}});
    assert(f.grid.get(&snapshot) == 3);
    f.grid.ageCells(getTns() - NSEC_SEC);
    f.grid.tick();
    assert(f.grid.get(&snapshot) == 0);
    assert(!f.grid.getCell(UUID128(0))); // Expiry remains active with no new input.
}

void testGridConfigLoadAndPersistence()
{
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}, {"nPminBuild", 0},
               {"vSelectedCells", {idHex(idForPath({7, 7}))}}}, true);
    f.publish({{{1.5f, 1.5f, 1.5f}, {1, 0, 0}, 0}});
    SELECTABLE_OCTGRID_CELLS before, after;
    assert(f.grid.get(&before) == 3);
    const json storedBefore = storedConfig(f);
    const json boundBefore = *f.config.getJson();
    json settings = gridConfig(f.grid);
    assert(settings.at("dTexpireCell") == "0" && settings.at("nMaxLevel") == "2");
    assert(f.grid.get(&after) == 3 && before.m_header.m_tStamp == after.m_header.m_tStamp);
    assert(f.grid.getCell(UUID128(0))->m_nP == 1 && f.grid.bSelectedOccupied(0));
    assert(storedConfig(f) == storedBefore); // Load never writes or rebuilds occupancy.
    assert(*f.config.getJson() == boundBefore);

    settings["vPorigin"] = {"0.125", "-0.5", "1.25"};
    settings["vRootCellSize"] = {"8", "10", "12"};
    settings["dTexpireCell"] = "18446744073709551615";
    settings["dTexpirePCL"] = "9007199254740993";
    settings["nMaxCells"] = "2147483647";
    settings["nMaxLevel"] = "4";
    settings["nPminBuild"] = "2147483647";
    const json reply = setGridConfig(f.grid, settings);
    assert(reply.at("bSuccess") == true && !reply.contains("bApplied"));
    const json current = gridConfig(f.grid);
    assert(current.at("dTexpireCell") == settings.at("dTexpireCell"));
    assert(current.at("dTexpirePCL") == settings.at("dTexpirePCL"));
    assert(current.at("nMaxCells") == settings.at("nMaxCells"));
    assert(current.at("nPminBuild") == settings.at("nPminBuild"));
    assert(f.grid.get(&after) == 0 && after.m_header.m_nMaxLevel == 4);
    assert(after.m_header.m_vPorigin == (std::array<float, 3>{.125f, -.5f, 1.25f}));
    assert(after.m_header.m_vRootCellSize == (std::array<float, 3>{8, 10, 12}));
    assert(storedConfig(f) == storedBefore && *f.config.getJson() == boundBefore);
    // Save ignores incoming drafts and persists only the current applied state.
    const json saveReply = gridCommand(f.grid, {{"cmd", "saveConfig"}, {"nMaxLevel", "30"},
        {"vPorigin", {"100", "100", "100"}}, {"cellIDs", json::array()}});
    assert(saveReply.at("bSuccess") == true && saveReply.size() == 4);
    assert(gridConfig(f.grid) == current);
    const json saved = storedConfig(f);
    assert(saved.at("dTexpireCell").is_number_unsigned() && saved.at("dTexpireCell").get<uint64_t>() == UINT64_MAX);
    assert(saved.at("dTexpirePCL").get<uint64_t>() == 9007199254740993ULL);
    assert(saved.at("nMaxCells") == INT32_MAX && saved.at("nPminBuild") == INT32_MAX);
    assert(saved.at("nMaxLevel") == 4 && saved.at("vPorigin") == json({.125f, -.5f, 1.25f}));
    Fixture restored(saved); // High cell cap must not allocate billions of cells at startup.
    assert(gridConfig(restored.grid) == current);
    assert(restored.grid.getSelectedCells().size() == f.grid.getSelectedCells().size());
}

void testInvalidGridConfigIsAtomic()
{
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}, {"nPminBuild", 0},
               {"vSelectedCells", {idHex(idForPath({7, 7}))}}}, true);
    f.publish({{{1.5f, 1.5f, 1.5f}, {1, 0, 0}, 0}});
    const json original = gridConfig(f.grid), saved = storedConfig(f), bound = *f.config.getJson();
    SELECTABLE_OCTGRID_CELLS before, after;
    assert(f.grid.get(&before) == 3);
    vector<json> invalid;
    for (const char *key : {"vPorigin", "vRootCellSize", "dTexpireCell", "dTexpirePCL", "nMaxCells", "nMaxLevel", "nPminBuild"})
    {
        json candidate = original;
        candidate.erase(key);
        invalid.push_back(candidate);
    }
    const vector<std::pair<string, json>> invalidValues = {
        {"vPorigin", {"1", "nan", "2"}}, {"vPorigin", {"0", "0", "1e9999"}},
        {"vPorigin", {"0", "0"}}, {"vPorigin", {0, 0, 0}},
        {"vRootCellSize", {"4", "0", "4"}}, {"vRootCellSize", {"4", "-1", "4"}},
        {"dTexpireCell", "18446744073709551616"}, {"dTexpirePCL", "-1"}, {"dTexpirePCL", "1e9"},
        {"nMaxCells", "2147483648"}, {"nMaxCells", "-1"}, {"nMaxCells", 20},
        {"nPminBuild", "2147483648"}, {"nPminBuild", "-1"}, {"nPminBuild", "1.5"},
        {"nMaxLevel", "41"}, {"nMaxLevel", "-1"}, {"nMaxLevel", "+1"}, {"nMaxLevel", " 1"},
        {"nMaxLevel", "1junk"}, {"nMaxLevel", ""}, {"nMaxLevel", false}, {"nMaxLevel", nullptr},
    };
    for (const auto &entry : invalidValues)
    {
        json candidate = original;
        candidate["vPorigin"] = {"1", "2", "3"}; // An early valid field must not apply before a later error.
        candidate[entry.first] = entry.second;
        invalid.push_back(candidate);
    }
    invalid.push_back({{"vPorigin", {"1", "2", "3"}}, {"vRootCellSize", {"8", "8", "8"}}});
    for (const auto &candidate : invalid)
    {
        const json reply = setGridConfig(f.grid, candidate);
        assert(reply.at("bSuccess") == false && reply.contains("error") && !reply.contains("bApplied"));
        assert(gridConfig(f.grid) == original && storedConfig(f) == saved);
        assert(*f.config.getJson() == bound);
        assert(f.grid.get(&after) == 3 && after.m_header.m_tStamp == before.m_header.m_tStamp);
        assert(f.grid.getCell(UUID128(0))->m_nP == 1 && f.grid.bSelectedOccupied(0));
    }
}

void testGridConfigRebuildsAndRemapsSelection()
{
    const UUID128 leaf = idForPath({7, 7}), parent = idForPath({7});
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}, {"nPminBuild", 0},
               {"vSelectedCells", {idHex(leaf)}}}, true);
    f.publish({{{1.5f, 1.5f, 1.5f}, {1, 0, 0}, 0}});
    json settings = gridConfig(f.grid);
    settings["nMaxLevel"] = "1";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    SELECTABLE_OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 0 && snapshot.m_header.m_nMaxLevel == 1);
    assert(!f.grid.getCell(UUID128(0)) && !f.grid.bSelectedOccupied(0));
    assert(f.grid.getSelectedCells().size() == 1 && sameID(f.grid.getSelectedCells().front(), parent));
    assert(storedConfig(f).at("vSelectedCells") == json::array({idHex(leaf)}));
    assert((*f.config.getJson())["grid"]["nMaxLevel"] == 2);
    assert(gridCommand(f.grid, {{"cmd", "saveConfig"}}).at("bSuccess") == true);
    assert(storedConfig(f).at("vSelectedCells") == json::array({idHex(parent)}));
    Fixture restored(storedConfig(f));
    assert(sameID(restored.grid.getSelectedCells().front(), parent));
    f.grid.tick(); // A geometry edit replays the current frame exactly once.
    assert(f.grid.get(&snapshot) == 2 && f.grid.getCell(UUID128(0))->m_nP == 1 && f.grid.bSelectedOccupied(0));
    f.grid.tick();
    assert(f.grid.getCell(UUID128(0))->m_nP == 1);

    GridReplyTransport transport;
    f.grid.jsonCmd({{"cmd", "setCellSelection"}, {"vPorigin", {"0", "0", "0"}},
                    {"vRootCellSize", {"4", "4", "4"}}, {"nMaxLevel", 2}, {"cellIDs", {idHex(leaf)}}}, &transport);
    assert(transport.replies.back().at("bSuccess") == false);
    assert(sameID(f.grid.getSelectedCells().front(), parent)); // Stale depth cannot enter persisted selection.

    settings["nMaxLevel"] = "3";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    assert(f.grid.get(&snapshot) == 0 && snapshot.m_header.m_nMaxLevel == 3);
    f.grid.tick();
    assert(f.grid.get(&snapshot) == 4 && sameID(f.grid.getSelectedCells().front(), parent));
    settings["vPorigin"] = {"1", "1", "1"};
    settings["vRootCellSize"] = {"2", "2", "2"};
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    assert(f.grid.getSelectedCells().size() == 1 && sameID(f.grid.getSelectedCells().front(), UUID128(0)));
    assert(storedConfig(f).at("vSelectedCells") == json::array({idHex(parent)}));
    assert(gridCommand(f.grid, {{"cmd", "saveConfig"}}).at("bSuccess") == true);
    assert(storedConfig(f).at("vSelectedCells") == json::array({idHex(UUID128(0))}));
    f.grid.jsonCmd({{"cmd", "setCellSelection"}, {"vPorigin", {"0", "0", "0"}},
                    {"vRootCellSize", {"4", "4", "4"}}, {"nMaxLevel", 3}, {"cellIDs", json::array()}}, &transport);
    assert(transport.replies.back().at("bSuccess") == false);
    assert(f.grid.getSelectedCells().size() == 1 && sameID(f.grid.getSelectedCells().front(), UUID128(0)));
    settings["nMaxLevel"] = "0";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    f.grid.tick();
    assert(f.grid.get(&snapshot) == 1 && snapshot.m_header.m_nMaxLevel == 0);
}

void testGridConfigRuntimeLimitsAndSaveFailure()
{
    Fixture f({{"nMaxLevel", 2}, {"nPminBuild", 0}}, true);
    const GEOMETRY_POINT point = {{1, 1, 1}, {1, 0, 0}, 0};
    f.publish({point, point, point});
    json settings = gridConfig(f.grid);
    SELECTABLE_OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 3);
    settings["nMaxCells"] = "1";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true && f.grid.get(&snapshot) == 1);
    settings["nPminBuild"] = "3";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true && f.grid.get(&snapshot) == 0);
    settings["nPminBuild"] = "2";
    settings["nMaxCells"] = "0";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true && f.grid.get(&snapshot) == 0);
    settings["nMaxCells"] = "10";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true && f.grid.get(&snapshot) == 3);
    assert(f.grid.getCell(UUID128(0))->m_nP == 3); // Publication updates never recount points.
    f.grid.ageCells(getTns() - NSEC_SEC);
    settings["dTexpireCell"] = "1";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true && f.grid.get(&snapshot) == 0);

    Fixture noFile({{"nMaxLevel", 2}});
    const json bound = *noFile.config.getJson();
    settings = gridConfig(noFile.grid);
    settings["nMaxLevel"] = "3";
    const json reply = setGridConfig(noFile.grid, settings);
    assert(reply.at("bSuccess") == true && !reply.contains("bApplied"));
    assert(*noFile.config.getJson() == bound);
    const json saveReply = gridCommand(noFile.grid, {{"cmd", "saveConfig"}});
    assert(saveReply.at("bSuccess") == false && saveReply.contains("error") && !saveReply.contains("bApplied"));
    assert(gridConfig(noFile.grid).at("nMaxLevel") == "3");
    assert(noFile.grid.get(&snapshot) == 0 && snapshot.m_header.m_nMaxLevel == 3);
}

void testCellSelectionApplyLoadAndSave()
{
    const string initialID = idHex(idForPath({7})), appliedID = idHex(idForPath({7, 7}));
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}, {"nPminBuild", 0},
               {"vSelectedCells", {initialID}}}, true);
    f.publish({{{1.5f, 1.5f, 1.5f}, {1, 0, 0}, 0}});
    const json diskBefore = storedConfig(f), boundBefore = *f.config.getJson();
    SELECTABLE_OCTGRID_CELLS before, after;
    assert(f.grid.get(&before) == 3);
    const json snapshotReply = gridCommand(f.grid, {{"cmd", "getCellSelection"}});
    const uint64_t selectionStamp = std::stoull(snapshotReply.at("tStamp").get<string>());
    assert(selectionStamp >= before.m_header.m_tStamp);
    json selection = cellSelection(f.grid);
    assert(selection.at("vPorigin") == json({"0.0", "0.0", "0.0"}));
    assert(selection.at("vRootCellSize") == json({"4.0", "4.0", "4.0"}));
    assert(selection.at("nMaxLevel").is_number_integer() && selection.at("nMaxLevel") == 2);
    assert(selection.at("cellIDs") == json::array({initialID}));
    selection["cellIDs"] = {appliedID};
    const json appliedReply = setCellSelection(f.grid, selection);
    assert(appliedReply.at("bSuccess") == true);
    assert(std::stoull(appliedReply.at("tStamp").get<string>()) >= selectionStamp);
    assert(cellSelection(f.grid) == selection && f.grid.bSelectedOccupied(0));
    assert(storedConfig(f) == diskBefore && *f.config.getJson() == boundBefore);
    assert(f.grid.get(&after) == 3 && after.m_header.m_tStamp == before.m_header.m_tStamp);

    const vector<std::pair<string, json>> invalidValues = {
        {"nMaxLevel", 1}, {"nMaxLevel", 3}, {"nMaxLevel", "2"}, {"nMaxLevel", 2.0},
        {"nMaxLevel", UINT64_MAX}, {"nMaxLevel", nullptr},
        {"vPorigin", {"1", "0", "0"}}, {"vRootCellSize", {"8", "4", "4"}},
        {"cellIDs", nullptr}, {"cellIDs", {appliedID, "invalid"}},
        {"cellIDs", {appliedID, idHex(idForPath({7, 7, 7}))}},
    };
    for (const auto &entry : invalidValues)
    {
        json candidate = selection;
        candidate[entry.first] = entry.second;
        const json reply = setCellSelection(f.grid, candidate);
        assert(reply.at("bSuccess") == false && reply.contains("error"));
        assert(cellSelection(f.grid) == selection);
    }
    json missingDepth = selection;
    missingDepth.erase("nMaxLevel");
    assert(setCellSelection(f.grid, missingDepth).at("bSuccess") == false);
    for (const char *oldCommand : {"octGridCellSelect", "loadCellSelect", "cellSelect"})
    {
        json oldRequest = selection;
        oldRequest["cmd"] = oldCommand;
        oldRequest["cellIDs"] = json::array();
        const json reply = gridCommand(f.grid, oldRequest);
        assert(reply.at("bSuccess") == false && reply.contains("error"));
        assert(cellSelection(f.grid) == selection);
    }
    assert(storedConfig(f) == diskBefore && *f.config.getJson() == boundBefore);
    assert(gridCommand(f.grid, {{"cmd", "saveConfig"}}).at("bSuccess") == true);
    assert(storedConfig(f).at("vSelectedCells") == json::array({appliedID}));
    Fixture restored(storedConfig(f));
    assert(cellSelection(restored.grid) == selection);

    const json savedBound = *f.config.getJson();
    selection["cellIDs"] = json::array();
    assert(setCellSelection(f.grid, selection).at("bSuccess") == true);
    assert(f.grid.getSelectedCells().empty() && !f.grid.bSelectedOccupied(0));
    assert(cellSelection(f.grid).at("cellIDs").empty());
    assert(storedConfig(f).at("vSelectedCells") == json::array({appliedID}) && *f.config.getJson() == savedBound);
    assert(gridCommand(f.grid, {{"cmd", "saveConfig"}}).at("bSuccess") == true);
    assert(storedConfig(f).at("vSelectedCells").empty());

    GridReplyTransport transport;
    f.grid.jsonCmd({{"cmd", "getCellSelection"}}, &transport);
    assert(!transport.replies.back().contains("requestId") && transport.replies.back().at("bSuccess") == true);
    f.grid.jsonCmd({{"cmd", "getGridConfig"}, {"requestId", 987654321}}, &transport);
    assert(transport.replies.back().at("requestId") == 987654321);
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
    SELECTABLE_OCTGRID_CELLS snapshot;
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
    testGridConfigLoadAndPersistence();
    testInvalidGridConfigIsAtomic();
    testGridConfigRebuildsAndRemapsSelection();
    testGridConfigRuntimeLimitsAndSaveFailure();
    testCellSelectionApplyLoadAndSave();
    testDenseSampleCloud();
    std::cout << "Octree grid regressions passed\n";
}
