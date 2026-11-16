#include "../../src/Universe/Grid/_SelectableOctGridPCL.h"
#include "../../src/Protocol/_JSONbase.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <limits>
#include <unistd.h>

using namespace kai;

void testOctreeGridBase();

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

class BackendHarness : public _OctreeGridPCL
{
public:
    void attach(PCLframe &frame) { m_vPointInputs.push_back({&frame, 0}); }
    void tick() { updateGrid(); }

    void ageCells(uint64_t stamp)
    {
        auto age = [stamp](auto &&self, OCTGRID_CELL<OCTGRID_CELL_PCL> *cell) -> void
        {
            if (!cell) return;
            if (auto *value = cell->getT()) value->m_tStamp = stamp;
            for (unsigned i = 0; i < N_OCT; ++i) self(self, cell->getChild(i));
        };
        age(age, m_pCell);
    }
};

class GridHarness : public _SelectableOctGridPCL
{
public:
    void attach(_OctreeGridPCL &grid) { m_pOGpcl = &grid; }
    void tick() { updateDrawAssets(); }
};

struct Fixture
{
    JsonCfg config;
    BackendHarness backend;
    GridHarness grid;
    PCLframe input;
    string configPath;

    explicit Fixture(json settings, bool persist = false)
    {
        json editor = {{"class", "_SelectableOctGridPCL"}, {"_OctreeGridPCL", "grid"}};
        for (const char *key : {"nMaxCells", "nPminBuild", "vColCellOcc"})
        {
            if (settings.contains(key)) editor[key] = settings[key];
            settings.erase(key);
        }
        settings["class"] = "_OctreeGridPCL";
        config.setJson({{"grid", std::move(settings)}, {"editor", std::move(editor)}});
        if (persist)
        {
            char path[] = "/tmp/openkai-grid-config-XXXXXX";
            const int fd = mkstemp(path);
            assert(fd >= 0);
            close(fd);
            configPath = path;
            assert(config.saveToFile(configPath) && config.readFromFile(configPath));
        }
        backend.setName("grid");
        backend.setConfig(&config, &(*config.getJson())["grid"]);
        assert(backend.loadConfig());
        backend.attach(input);
        grid.setName("editor");
        grid.setConfig(&config, &(*config.getJson())["editor"]);
        assert(grid.loadConfig());
        grid.attach(backend);
        grid.tick();
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
        tick();
    }

    void tick()
    {
        backend.tick();
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
    command["module"] = "editor";
    command["requestId"] = "grid-test";
    grid.jsonCmd(command, &transport);
    assert(transport.replies.size() == 1);
    const json reply = transport.replies.front();
    assert(reply.at("cmd") == command.at("cmd") && reply.at("module") == "editor");
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
    const json &backend = saved.getJson()->at("grid"), &editor = saved.getJson()->at("editor");
    assert(!editor.contains("vSelectedCells") && !editor.contains("nMaxLevel"));
    assert(!backend.contains("nMaxCells") && !backend.contains("nPminBuild"));
    json settings = backend;
    for (const char *key : {"nMaxCells", "nPminBuild", "vColCellOcc"})
        if (editor.contains(key)) settings[key] = editor[key];
    return settings;
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
    const auto *root = f.backend.getCell(UUID128(0));
    assert(root && root->getT()->m_nP == 16);
    assert((root->getT()->m_vC - Vector4f(.5f, 0, .5f, 1)).norm() < 1e-6f);
    SELECTABLE_OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 17); // Root, eight parents, eight leaves.
    for (unsigned octant = 0; octant < 8; ++octant)
    {
        const auto &position = points[octant * 2].m_vP;
        const auto *leaf = f.backend.getCell(position);
        assert(leaf && leaf->getT()->m_nP == 2 && f.backend.getCell(leaf->m_ID) == leaf);
        assert(leaf->m_ID.m_uint64[0] == 0);
        assert(leaf->m_ID.m_uint64[1] == (uint64_t(2) << 56 | uint64_t(octant) << 53 | uint64_t(octant) << 50));
        assert((leaf->getT()->m_vC - Vector4f(.5f, 0, .5f, 1)).norm() < 1e-6f);
        std::array<float, 3> center, size;
        assert(octgridCellBox(snapshot.m_header, leaf->m_ID, center, size));
        for (unsigned axis = 0; axis < 3; ++axis)
            assert(center[axis] == position[axis] && size[axis] == 1);
    }

    f.tick();
    assert(f.backend.getCell(UUID128(0))->getT()->m_nP == 16); // No recount of an unchanged frame.
    f.publish({{points.front().m_vP, {0, 1, 0}, 0}});
    const auto *leaf = f.backend.getCell(points.front().m_vP);
    assert(leaf && leaf->getT()->m_nP == 3);
    assert((leaf->getT()->m_vC - Vector4f(1.f / 3, 1.f / 3, 1.f / 3, 1)).norm() < 1e-6f);
    assert(!f.backend.addCellPoint({{2.01f, 0, 0}, {1, 1, 1}, getTns()}, getTns()));
    assert(f.backend.getCell(UUID128(0))->getT()->m_nP == 17);
}

void testSingleSampleFilters()
{
    Median<int> median;
    Average<int64_t> average;
    assert(median.init(3) && average.init(3));
    for (int value : {2, 8, 5})
    {
        median.update(value);
        average.update(value);
    }
    assert(median.get() == 5 && average.get() == 5);

    for (int window : {1, 0, -1})
    {
        assert(median.init(window) && average.init(window));
        for (int value : {9, 1, -7, INT32_MAX})
        {
            assert(median.update(value) == value && median.get() == value);
            assert(average.update(value) == value && average.get() == value);
        }
        median.reset();
        average.reset();
        assert(median.get() == 0 && average.get() == 0);
        assert(median.update(4) == 4 && average.update(4) == 4);
    }

    // Re-enabling smoothing starts with fresh histories, including window 2.
    assert(median.init(2) && average.init(2));
    assert(median.update(8) == 8 && average.update(8) == 8);
    assert(median.update(2) == 8 && average.update(2) == 5);
    assert(median.update(4) == 4 && average.update(4) == 3);
}

void testPointFilterBypass()
{
    const GEOMETRY_POINT point = {{1, 1, 1}, {1, 0, 0}, 0};
    const json windows[] = {json::object(), {{"nWfilMed", 1}, {"nWfilAvr", 3}},
                           {{"nWfilMed", 3}, {"nWfilAvr", 1}}};
    const int expected[][3] = {{2, 8, 20}, {2, 5, 10}, {2, 8, 8}};
    for (size_t mode = 0; mode < std::size(windows); ++mode)
    {
        json settings = windows[mode];
        settings["nMaxLevel"] = 0;
        settings["vSelectedCells"] = {idHex(UUID128(0))};
        Fixture f(settings);
        const int added[] = {2, 6, 12};
        for (size_t i = 0; i < std::size(added); ++i)
        {
            f.publish(vector<GEOMETRY_POINT>(added[i], point));
            assert(f.backend.getCell(UUID128(0))->getT()->m_nPfiltered == expected[mode][i]);
            assert(f.backend.bSelectedOccupied(7) == (expected[mode][i] > 7));
        }
        assert(f.backend.saveConfig(false));
        const json &saved = f.config.getJson()->at("grid");
        assert(saved.at("nWfilMed") == windows[mode].value("nWfilMed", 1));
        assert(saved.at("nWfilAvr") == windows[mode].value("nWfilAvr", 1));
    }
}

void testPointFilterWindowsAndOccupancy()
{
    const UUID128 leafID = idForPath({7, 7});
    const GEOMETRY_POINT point = {{1.5f, 1.5f, 1.5f}, {1, 0, 0}, 0};
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}, {"nPminBuild", 21},
               {"nWfilMed", 5}, {"nWfilAvr", 3}, {"vSelectedCells", {idHex(leafID)}}}, true);
    const int counts[] = {2, 8, 20, 26, 32, 38};
    const int filtered[] = {2, 5, 10, 18, 22, 24};
    int previous = 0;
    for (size_t i = 0; i < std::size(counts); ++i)
    {
        f.publish(vector<GEOMETRY_POINT>(counts[i] - previous, point));
        previous = counts[i];
        for (int level = 0; level <= 2; ++level)
        {
            const auto *cell = f.backend.getCell(point.m_vP, level);
            assert(cell && cell->getT()->m_nP == counts[i]);
            assert(cell->getT()->m_nPfiltered == filtered[i]);
        }
        assert(f.backend.bSelectedOccupied(21) == (filtered[i] > 21));
        assert(!f.backend.bSelectedOccupied(filtered[i])); // Exclusive threshold.
        SELECTABLE_OCTGRID_CELLS snapshot;
        assert(f.grid.get(&snapshot) == (filtered[i] > 21 ? 3 : 0));
        f.grid.tick(); // Publishing alone must not advance the time window.
        assert(f.backend.getCell(leafID)->getT()->m_nPfiltered == filtered[i]);
    }
    f.tick(); // Unchanged input still advances filters once per backend update.
    assert(f.backend.getCell(leafID)->getT()->m_nPfiltered == 26);
    assert(f.backend.getCell(leafID)->getT()->m_nP == 38);
    for (int i = 0; i < 5; ++i) f.tick();
    assert(f.backend.getCell(leafID)->getT()->m_nPfiltered == 38);

    assert(f.backend.saveConfig(true));
    const json saved = storedConfig(f);
    assert(saved.at("nWfilMed") == 5 && saved.at("nWfilAvr") == 3);
    assert(f.backend.loadConfig()); // Reload recreates cells with fresh filter histories.
    f.publish({point, point});
    assert(f.backend.getCell(leafID)->getT()->m_nPfiltered == 2);
    f.publish(vector<GEOMETRY_POINT>(6, point));
    f.publish(vector<GEOMETRY_POINT>(12, point));
    assert(f.backend.getCell(leafID)->getT()->m_nPfiltered == 10);

    Fixture minimum({{"nWfilMed", -1}, {"nWfilAvr", 0}});
    assert(minimum.backend.saveConfig(false));
    const json &effective = minimum.config.getJson()->at("grid");
    assert(effective.at("nWfilMed") == 1 && effective.at("nWfilAvr") == 1);
}

void testPointFilterLargeCountsAndExpiry()
{
    const UUID128 leafID = idForPath({7, 7});
    const GEOMETRY_POINT point = {{1.5f, 1.5f, 1.5f}, {1, 0, 0}, 0};
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}, {"nPminBuild", 0},
               {"nWfilMed", 3}, {"nWfilAvr", 2},
               {"dTexpireCell", NSEC_SEC}, {"vSelectedCells", {idHex(leafID)}}});
    f.publish({point});
    auto *payload = f.backend.getCell(leafID)->getT();
    payload->m_nP = INT32_MAX;
    for (int i = 0; i < 4; ++i) f.tick();
    assert(payload->m_nPfiltered == INT32_MAX); // The average sum exceeds int32.

    // A structural parent without a payload must not hide descendant filters.
    auto *root = f.backend.getCell(UUID128(0));
    delete root->m_pT;
    root->m_pT = nullptr;
    payload->clear();
    assert(payload->m_nPfiltered == 0 && payload->m_filMed.get() == 0 && payload->m_filAvr.get() == 0);
    payload->m_nP = 1;
    payload->m_tStamp = getTns();
    f.tick();
    assert(payload->m_nPfiltered == 1 && f.backend.bSelectedOccupied(0));

    f.backend.ageCells(getTns() - 2 * NSEC_SEC);
    f.tick();
    assert(!f.backend.getCell(leafID) && !f.backend.bSelectedOccupied(0));
    f.publish({point});
    assert(f.backend.getCell(leafID)->getT()->m_nPfiltered == 1);
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
    assert(f.backend.getSelectedCells().size() == 1);
    assert(sameID(f.backend.getSelectedCells().front(), selectedID));
    assert(!f.backend.bSelectedOccupied(0));
    // The positive root corner chooses child 7 at every depth, including where
    // float cell centers round to the input point near the deepest levels.
    const Vector3f position(1, 1, 1);
    f.publish({{position, {.2f, .4f, .6f}, 0}});
    SELECTABLE_OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 41);
    for (unsigned depth = 0; depth <= 40; ++depth)
    {
        const UUID128 id = idForPath(vector<unsigned>(depth, 7));
        const auto *cell = f.backend.getCell(position, depth);
        assert(cell && sameID(cell->m_ID, id));
        assert(f.backend.getCell(id) == cell);
        assert(sameID(snapshot.m_vCell[depth].id(), id));
    }
    assert(f.backend.bSelectedOccupied(0));
    assert(!f.backend.bSelectedOccupied(1));
    assert(f.grid.saveConfig(false));
    assert((*f.config.getJson())["grid"]["vSelectedCells"] == json::array({selectedHex}));
    assert(f.backend.loadConfig());
    assert(sameID(f.backend.getSelectedCells().front(), selectedID));
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
        assert(!f.backend.getCell(id));
        const UUID128 selected = f.backend.getSelectedCells().front();
        f.grid.jsonCmd({{"cmd", "setCellSelection"}, {"vPorigin", {"0", "0", "0"}},
                        {"vRootCellSize", {"2", "2", "2"}}, {"nMaxLevel", 40}, {"cellIDs", {idHex(id)}}}, nullptr);
        assert(f.backend.getSelectedCells().size() == 1 && sameID(f.backend.getSelectedCells().front(), selected));
        (*f.config.getJson())["grid"]["vSelectedCells"] = json::array({idHex(id)});
        assert(!f.backend.loadConfig());
        (*f.config.getJson())["grid"]["vSelectedCells"] = json::array({idHex(selected)});
        assert(f.backend.loadConfig());
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
    f.tick();
    SELECTABLE_OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 0);
    f.publish({{{0, 0, 1}, {.25f, .5f, .75f}, 0}});
    assert(f.grid.get(&snapshot) == 3);
    f.backend.ageCells(getTns() - NSEC_SEC);
    f.tick();
    assert(f.grid.get(&snapshot) == 0);
    assert(!f.backend.getCell(UUID128(0))->getT()); // Expiry remains active with no new input.
}

void testPayloadClearPreservesCellIdentity()
{
    const UUID128 leafID = idForPath({7, 7});
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}, {"nPminBuild", 0},
               {"vSelectedCells", {idHex(leafID)}}});
    f.publish({{{1.5f, 1.5f, 1.5f}, {1, 0, 0}, 0}});
    auto *cell = f.backend.getCell(leafID);
    assert(cell && f.backend.bSelectedOccupied(0));
    cell->getT()->clear();
    assert(sameID(cell->m_ID, leafID) && f.backend.getCell(leafID) == cell);
    assert(!f.backend.bSelectedOccupied(-1));
    cell->getT()->m_tStamp = getTns();
    assert(!f.backend.bSelectedOccupied(-1)); // A timestamp alone is not occupancy.
    f.publish({{{1.5f, 1.5f, 1.5f}, {0, 1, 0}, 0}});
    assert(f.backend.getCell(leafID) == cell && cell->getT()->m_nP == 1);
    assert(f.backend.bSelectedOccupied(0));
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
    assert(settings.at("nWfilMed") == "1" && settings.at("nWfilAvr") == "1");
    assert(f.grid.get(&after) == 3 && before.m_header.m_tStamp == after.m_header.m_tStamp);
    assert(f.backend.getCell(UUID128(0))->getT()->m_nP == 1 && f.backend.bSelectedOccupied(0));
    assert(storedConfig(f) == storedBefore); // Load never writes or rebuilds occupancy.
    assert(*f.config.getJson() == boundBefore);

    settings["vPorigin"] = {"0.125", "-0.5", "1.25"};
    settings["vRootCellSize"] = {"8", "10", "12"};
    settings["dTexpireCell"] = "18446744073709551615";
    settings["dTexpirePCL"] = "9007199254740993";
    settings["nMaxCells"] = "2147483647";
    settings["nMaxLevel"] = "4";
    settings["nPminBuild"] = "2147483647";
    settings["nWfilMed"] = "2147483647";
    settings["nWfilAvr"] = "2147483647";
    const json reply = setGridConfig(f.grid, settings);
    assert(reply.at("bSuccess") == true && !reply.contains("bApplied"));
    const json current = gridConfig(f.grid);
    assert(current.at("dTexpireCell") == settings.at("dTexpireCell"));
    assert(current.at("dTexpirePCL") == settings.at("dTexpirePCL"));
    assert(current.at("nMaxCells") == settings.at("nMaxCells"));
    assert(current.at("nPminBuild") == settings.at("nPminBuild"));
    assert(current.at("nWfilMed") == settings.at("nWfilMed"));
    assert(current.at("nWfilAvr") == settings.at("nWfilAvr"));
    assert(f.grid.get(&after) == 0 && after.m_header.m_nMaxLevel == 4);
    assert(after.m_header.m_vPorigin == (std::array<float, 3>{.125f, -.5f, 1.25f}));
    assert(after.m_header.m_vRootCellSize == (std::array<float, 3>{8, 10, 12}));
    assert(storedConfig(f) == storedBefore && *f.config.getJson() == boundBefore);
    // Save ignores incoming drafts and persists only the current applied state.
    const json saveReply = gridCommand(f.grid, {{"cmd", "saveConfig"}, {"nMaxLevel", "30"},
        {"nWfilMed", "1"}, {"nWfilAvr", "1"},
        {"vPorigin", {"100", "100", "100"}}, {"cellIDs", json::array()}});
    assert(saveReply.at("bSuccess") == true && saveReply.size() == 4);
    assert(gridConfig(f.grid) == current);
    const json saved = storedConfig(f);
    assert(saved.at("dTexpireCell").is_number_unsigned() && saved.at("dTexpireCell").get<uint64_t>() == UINT64_MAX);
    assert(saved.at("dTexpirePCL").get<uint64_t>() == 9007199254740993ULL);
    assert(saved.at("nMaxCells") == INT32_MAX && saved.at("nPminBuild") == INT32_MAX);
    assert(saved.at("nWfilMed") == INT32_MAX && saved.at("nWfilAvr") == INT32_MAX);
    assert(saved.at("nMaxLevel") == 4 && saved.at("vPorigin") == json({.125f, -.5f, 1.25f}));
    Fixture restored(saved); // High cell cap must not allocate billions of cells at startup.
    assert(gridConfig(restored.grid) == current);
    assert(restored.backend.getSelectedCells().size() == f.backend.getSelectedCells().size());
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
    for (const char *key : {"vPorigin", "vRootCellSize", "dTexpireCell", "dTexpirePCL", "nMaxCells", "nMaxLevel", "nPminBuild", "nWfilMed", "nWfilAvr"})
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
    json oldConfig = original;
    oldConfig.erase("nWfilMed");
    oldConfig.erase("nWfilAvr");
    invalid.push_back(oldConfig);
    for (const char *key : {"nWfilMed", "nWfilAvr"})
        for (const json &value : json::array({"0", "-1", "2147483648", "1.5", "1e2", "", 1, false, nullptr}))
        {
            json candidate = original;
            candidate["nWfilMed"] = "5"; // Invalid average must not apply the valid median window.
            candidate[key] = value;
            invalid.push_back(candidate);
        }
    for (const auto &candidate : invalid)
    {
        const json reply = setGridConfig(f.grid, candidate);
        assert(reply.at("bSuccess") == false && reply.contains("error") && !reply.contains("bApplied"));
        assert(gridConfig(f.grid) == original && storedConfig(f) == saved);
        assert(*f.config.getJson() == bound);
        assert(f.grid.get(&after) == 3 && after.m_header.m_tStamp == before.m_header.m_tStamp);
        assert(f.backend.getCell(UUID128(0))->getT()->m_nP == 1 && f.backend.bSelectedOccupied(0));
    }
}

void testGridConfigUpdatesLiveFilters()
{
    const GEOMETRY_POINT point = {{1.5f, 1.5f, 1.5f}, {1, 0, 0}, 0};
    const UUID128 leafID = idForPath({7, 7});
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}, {"nPminBuild", 5},
               {"vSelectedCells", {idHex(leafID)}}}, true);
    f.publish({point, point});
    auto *leaf = f.backend.getCell(leafID);
    const auto stamp = leaf->getT()->m_tStamp;
    const json bound = *f.config.getJson(), saved = storedConfig(f);
    json settings = gridConfig(f.grid);
    settings["nWfilMed"] = "3";
    settings["nWfilAvr"] = "3";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    assert(f.backend.getCell(leafID) == leaf && leaf->getT()->m_tStamp == stamp);
    assert(leaf->getT()->m_nP == 2 && leaf->getT()->m_nPfiltered == 2);
    assert(sameID(f.backend.getSelectedCells().front(), leafID));
    f.publish(vector<GEOMETRY_POINT>(6, point)); // 2 -> 8 yields average 5.
    for (int level = 0; level <= 2; ++level)
        assert(f.backend.getCell(point.m_vP, level)->getT()->m_nPfiltered == 5);
    SELECTABLE_OCTGRID_CELLS snapshot;
    assert(f.grid.get(&snapshot) == 0 && !f.backend.bSelectedOccupied(5));
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    assert(gridConfig(f.grid) == settings && leaf->getT()->m_nPfiltered == 5); // Unchanged Apply/Load do not sample.

    settings["nWfilAvr"] = "1"; // Reset existing histories and seed from raw count 8.
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    assert(f.grid.get(&snapshot) == 3 && f.backend.bSelectedOccupied(5));
    assert(leaf->getT()->m_nP == 8 && leaf->getT()->m_nPfiltered == 8);
    f.publish(vector<GEOMETRY_POINT>(6, point)); // Median warmup passes 14 through.
    f.publish(vector<GEOMETRY_POINT>(6, point)); // Full median [8,14,20] produces 14.
    assert(leaf->getT()->m_nP == 20 && leaf->getT()->m_nPfiltered == 14);
    settings["nWfilMed"] = "1";
    settings["nWfilAvr"] = "3";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    assert(leaf->getT()->m_nPfiltered == 20);
    f.publish(vector<GEOMETRY_POINT>(6, point));
    assert(leaf->getT()->m_nPfiltered == 23);
    settings["nWfilAvr"] = "1";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    assert(leaf->getT()->m_nPfiltered == 26 && f.backend.getCell(leafID) == leaf);
    assert(*f.config.getJson() == bound && storedConfig(f) == saved);

    // Newly allocated cells inherit applied windows, not the launch JSON values.
    settings["nWfilAvr"] = "3";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    GEOMETRY_POINT other = point;
    other.m_vP = {-1.5f, -1.5f, -1.5f};
    f.publish({other, other});
    f.publish(vector<GEOMETRY_POINT>(6, other));
    assert(f.backend.getCell(other.m_vP)->getT()->m_nPfiltered == 5);
    assert(gridCommand(f.grid, {{"cmd", "saveConfig"}}).at("bSuccess") == true);
    Fixture restored(storedConfig(f));
    assert(gridConfig(restored.grid) == settings);
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
    assert(!f.backend.getCell(UUID128(0))->getT() && !f.backend.bSelectedOccupied(0));
    assert(f.backend.getSelectedCells().size() == 1 && sameID(f.backend.getSelectedCells().front(), parent));
    assert(storedConfig(f).at("vSelectedCells") == json::array({idHex(leaf)}));
    assert((*f.config.getJson())["grid"]["nMaxLevel"] == 2);
    assert(gridCommand(f.grid, {{"cmd", "saveConfig"}}).at("bSuccess") == true);
    assert(storedConfig(f).at("vSelectedCells") == json::array({idHex(parent)}));
    Fixture restored(storedConfig(f));
    assert(sameID(restored.backend.getSelectedCells().front(), parent));
    f.tick(); // A geometry edit replays the current frame exactly once.
    assert(f.grid.get(&snapshot) == 2 && f.backend.getCell(UUID128(0))->getT()->m_nP == 1 && f.backend.bSelectedOccupied(0));
    f.tick();
    assert(f.backend.getCell(UUID128(0))->getT()->m_nP == 1);

    GridReplyTransport transport;
    f.grid.jsonCmd({{"cmd", "setCellSelection"}, {"vPorigin", {"0", "0", "0"}},
                    {"vRootCellSize", {"4", "4", "4"}}, {"nMaxLevel", 2}, {"cellIDs", {idHex(leaf)}}}, &transport);
    assert(transport.replies.back().at("bSuccess") == false);
    assert(sameID(f.backend.getSelectedCells().front(), parent)); // Stale depth cannot enter persisted selection.

    settings["nMaxLevel"] = "3";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    assert(f.grid.get(&snapshot) == 0 && snapshot.m_header.m_nMaxLevel == 3);
    f.tick();
    assert(f.grid.get(&snapshot) == 4 && sameID(f.backend.getSelectedCells().front(), parent));
    settings["vPorigin"] = {"1", "1", "1"};
    settings["vRootCellSize"] = {"2", "2", "2"};
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    assert(f.backend.getSelectedCells().size() == 1 && sameID(f.backend.getSelectedCells().front(), UUID128(0)));
    assert(storedConfig(f).at("vSelectedCells") == json::array({idHex(parent)}));
    assert(gridCommand(f.grid, {{"cmd", "saveConfig"}}).at("bSuccess") == true);
    assert(storedConfig(f).at("vSelectedCells") == json::array({idHex(UUID128(0))}));
    f.grid.jsonCmd({{"cmd", "setCellSelection"}, {"vPorigin", {"0", "0", "0"}},
                    {"vRootCellSize", {"4", "4", "4"}}, {"nMaxLevel", 3}, {"cellIDs", json::array()}}, &transport);
    assert(transport.replies.back().at("bSuccess") == false);
    assert(f.backend.getSelectedCells().size() == 1 && sameID(f.backend.getSelectedCells().front(), UUID128(0)));
    settings["nMaxLevel"] = "0";
    assert(setGridConfig(f.grid, settings).at("bSuccess") == true);
    f.tick();
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
    assert(f.backend.getCell(UUID128(0))->getT()->m_nP == 3); // Publication updates never recount points.
    f.backend.ageCells(getTns() - NSEC_SEC);
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
    assert(cellSelection(f.grid) == selection && f.backend.bSelectedOccupied(0));
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
    assert(f.backend.getSelectedCells().empty() && !f.backend.bSelectedOccupied(0));
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

void testRuntimeWithoutEditor()
{
    Fixture f({{"vRootCellSize", {4, 4, 4}}, {"nMaxLevel", 2}}, true);
    json selection = cellSelection(f.grid);
    const UUID128 selected = idForPath({7, 7});
    selection["cellIDs"] = {idHex(selected)};
    assert(setCellSelection(f.grid, selection).at("bSuccess") == true);
    assert(gridCommand(f.grid, {{"cmd", "saveConfig"}}).at("bSuccess") == true);

    // Restart the saved launch file with all editor/viewer instances disabled.
    JsonCfg launch;
    assert(launch.readFromFile(f.configPath));
    json &config = *launch.getJson();
    config["editor"]["bON"] = false;
    config["viewer"] = {{"class", "_WebSelectableOctGrid"}, {"bON", false}};
    config["commands"] = {{"class", "_JSONcmd"}, {"bON", false}};
    config["points"] = {{"type", "dataObject"}, {"class", "PCLframe"}};
    config["grid"]["bON"] = true;
    config["grid"]["vPCLframesIn"] = {"points"};
    config["grid"]["thread"]["FPS"] = 100;
    assert(launch.saveToFile());

    InstanceMgr instances;
    assert(instances.loadJsonFiles(f.configPath) && instances.createAll());
    assert(!instances.findModule("editor") && !instances.findModule("viewer") && !instances.findModule("commands"));
    assert(instances.initAll() && instances.linkAll());
    auto *backend = static_cast<_OctreeGridPCL *>(instances.findModule("grid"));
    auto *points = static_cast<PCLframe *>(instances.findDataObject("points"));
    assert(backend && points && backend->getSelectedCells().size() == 1);
    assert(sameID(backend->getSelectedCells().front(), selected));
    assert(!backend->bSelectedOccupied(0));
    // Returned selections are snapshots, not mutable aliases into the runtime grid.
    auto copy = backend->getSelectedCells();
    copy.clear();
    assert(backend->getSelectedCells().size() == 1);
    const auto stamp = getTns();
    points->set({{{1.5f, 1.5f, 1.5f}, {1, 0, 0}, stamp}}, stamp);
    assert(instances.startAll());
    const auto deadline = getTns() + 2 * NSEC_SEC;
    while (!backend->bSelectedOccupied(0) && getTns() < deadline) usleep(1000);
    assert(backend->bSelectedOccupied(0) && !backend->bSelectedOccupied(1));
    assert(backend->saveConfig(true));
    assert(launch.readFromFile(f.configPath));
    assert(launch.getJson()->at("grid").at("vSelectedCells") == json::array({idHex(selected)}));
    assert(!launch.getJson()->at("editor").contains("vSelectedCells"));
    instances.stopAll();
    instances.cleanAll();
}

void testStoppingEditorLeavesBackendRunning()
{
    Fixture f({{"nMaxLevel", 1}, {"nPminBuild", 0}, {"vSelectedCells", {idHex(UUID128(0))}}});
    auto publish = [&]()
    {
        const auto stamp = getTns();
        f.input.set({{{1, 1, 1}, {1, 0, 0}, stamp}}, stamp);
    };
    assert(f.backend.start() && f.grid.start());
    publish();
    SELECTABLE_OCTGRID_CELLS snapshot;
    auto deadline = getTns() + 2 * NSEC_SEC;
    while (f.grid.get(&snapshot) == 0 && getTns() < deadline) usleep(1000);
    assert(f.grid.get(&snapshot) == 2 && f.backend.bSelectedOccupied(0));
    f.grid.stop(); // Join the editor worker before its linked backend can be released.
    f.grid.get(&snapshot);
    const auto lastDraw = snapshot.m_header.m_tStamp;
    publish();
    deadline = getTns() + 2 * NSEC_SEC;
    while (!f.backend.bSelectedOccupied(1) && getTns() < deadline) usleep(1000);
    assert(f.backend.bSelectedOccupied(1));
    f.grid.get(&snapshot);
    assert(snapshot.m_header.m_tStamp == lastDraw);
}

void testEditorLinkValidation()
{
    JsonCfg config;
    config.setJson({{"editor", {{"class", "_SelectableOctGridPCL"}, {"_OctreeGridPCL", "missing"}}}});
    _SelectableOctGridPCL editor;
    editor.setName("editor");
    editor.setConfig(&config, &(*config.getJson())["editor"]);
    InstanceMgr instances;
    assert(editor.loadConfig() && !editor.link(&instances) && !editor.start());
    GridReplyTransport transport;
    editor.jsonCmd({{"cmd", "getCellSelection"}}, &transport);
    assert(transport.replies.size() == 1 && transport.replies.back().at("bSuccess") == false);
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
    const auto *root = f.backend.getCell(UUID128(0));
    assert(root && root->getT()->m_nP == 640 * 480);
    assert((root->getT()->m_vC - Vector4f(.25f, .5f, .75f, 1)).norm() < 1e-6f);
    assert(snapshot.m_header.m_nMaxLevel == 5);
    assert(snapshot.m_header.m_vRootCellSize == (std::array<float, 3>{5, 5, 5}));
    std::cout << "Dense sample cloud: " << snapshot.m_vCell.size()
              << " occupied cells in " << elapsedMs << " ms\n";
}
}

int main()
{
    testOctreeGridBase();
    testCountsColorsAndPaths();
    testSingleSampleFilters();
    testPointFilterBypass();
    testPointFilterWindowsAndOccupancy();
    testPointFilterLargeCountsAndExpiry();
    testCellIDLayoutAndGeometry();
    testDeepGeneratedIDsAndSelection();
    testInvalidCellIDs();
    testStaleAndInvalidPoints();
    testPayloadClearPreservesCellIdentity();
    testGridConfigLoadAndPersistence();
    testInvalidGridConfigIsAtomic();
    testGridConfigUpdatesLiveFilters();
    testGridConfigRebuildsAndRemapsSelection();
    testGridConfigRuntimeLimitsAndSaveFailure();
    testCellSelectionApplyLoadAndSave();
    testRuntimeWithoutEditor();
    testStoppingEditorLeavesBackendRunning();
    testEditorLinkValidation();
    testDenseSampleCloud();
    std::cout << "Octree grid regressions passed\n";
}
