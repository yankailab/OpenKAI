#include "../../src/3D/Grid/_OctreeGrid.h"
#include "../../src/3D/Viewer/_ImGUIviewer.h"
#include "../../src/3D/PointCloud/_PointCloud.h"
#include "../../src/3D/Line/_Line.h"
#include "../../src/UI/_WSconsole.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace kai;
class Grid : public _OctreeGrid {
public:
    using _OctreeGrid::updateDrawAssets;
    using _OctreeGrid::deleteExpiredCells;
    vector<UUID128> selectedCells() {
        std::lock_guard<std::mutex> lock(m_cellsMutex);
        return m_vSelectedCells;
    }
    void age(OCTREE_CELL<OCTGRID_PCL_CELL>* cell = nullptr) {
        if (!cell) cell = m_pCell;
        if (cell->getT()) cell->getT()->m_tStamp = 1;
        for (auto child : cell->m_pChild) if (child) age(child);
    }
};
class SelectionConsole : public _WSconsole {
public:
    json reply;
    using _WSconsole::handleJson;
    void source(BASE *grid) { m_vpB = {grid}; }
    bool sendJson(const json &j) override { reply = j; return true; }
};

void checkSelectionCommand() {
    Grid grid;
    grid.setName("octGrid");
    assert(grid.init(json{{"class","_OctreeGrid"}, {"thread",{{"FPS",30}}}, {"nP",1},
        {"nMaxLevel",40}, {"nMaxCells",0}, {"vPorigin",{.1f,-.2f,1e-6f}}, {"vRootCellSize",{8,4,2}}}));
    SelectionConsole console;
    console.source(&grid);
    const string root(32, '0');
    const string deep = "e8cdab8967452301efcdab8967452321";
    const json command = {{"cmd","octGridCellSelect"}, {"module","octGrid"},
        {"vPorigin",{"0.10000000149011612","-0.20000000298023224","9.999999974752427e-7"}},
        {"vRootCellSize",{"8","4","2"}}, {"cellIDs",{root,deep}}};
    auto send = [&](const json &j, bool success) {
        console.reply = nullptr;
        console.handleJson(j.dump()); // Actual _WSconsole JSON parsing and module dispatch.
        assert(console.reply == json({{"cmd","octGridCellSelect"}, {"bSuccess",success}}));
    };
    auto checkIDs = [&] {
        const auto ids = grid.selectedCells();
        assert(ids.size() == 2 && ids[0] == uint64_t(0));
        assert(ids[1].m_uint64[0] == 0x0123456789abcde8ULL && ids[1].m_uint64[1] == 0x2123456789abcdefULL);
        assert(!grid.getCell(ids[1])); // Retained selections need not be occupied.
    };
    send(command, true); checkIDs();
    auto upper = command;
    upper["cellIDs"][1] = "E8CDAB8967452301EFCDAB8967452321";
    send(upper, true); checkIDs();

    // A malformed item after a valid one must not partially replace the list.
    const vector<json> badIDs = {nullptr, 7, "", string(31, '0'), string(33, '0'),
        "zz000000000000000000000000000000", // invalid hexadecimal
        "29000000000000000000000000000000", // level 41
        "00000000000000000000000000000080", // reserved high bits
        "40000000000000000000000000000000"}; // unused path bit on root
    for (const auto &id : badIDs) {
        auto bad = command; bad["cellIDs"] = json::array({root,id});
        send(bad, false); checkIDs();
    }
    for (const auto &field : vector<json>{nullptr, 7, root, json::object()}) {
        auto bad = command; bad["cellIDs"] = field;
        send(bad, false); checkIDs();
    }
    auto missing = command; missing.erase("cellIDs");
    send(missing, false); checkIDs();

    // Stale or invalid root information clears selections, without accepting IDs.
    for (const string field : {"vPorigin", "vRootCellSize"}) {
        for (const auto &value : vector<json>{"0", "NaN", "inf", "1e999", "8m", "", 8}) {
            send(command, true);
            auto stale = command; stale[field][0] = value;
            send(stale, false); assert(grid.selectedCells().empty());
        }
        send(command, true);
        auto absent = command; absent.erase(field);
        send(absent, false); assert(grid.selectedCells().empty());
        send(command, true);
        auto shortHeader = command; shortHeader[field] = json::array({"0","0"});
        send(shortHeader, false); assert(grid.selectedCells().empty());
    }
    send(command, true);
    auto empty = command; empty["cellIDs"] = json::array();
    send(empty, true); assert(grid.selectedCells().empty());
    grid.console(command, nullptr); checkIDs();
    grid.console(json{{"cmd","unrelated"}}, nullptr); checkIDs();
    std::cout << "PASS: _WSconsole selection dispatch, exact 128-bit IDs, decimal headers, stale-grid clearing, atomic validation and replies\n";
}

void checkSelectionConfig() {
    char pattern[] = "/tmp/openkai-grid-config-XXXXXX";
    const char *directory = mkdtemp(pattern);
    assert(directory);
    const std::filesystem::path folder(directory);
    const string file = (folder / "grid.json").string(), alternate = (folder / "alternate.json").string();
    const string root(32, '0'), deep = "e8cdab8967452301efcdab8967452321";
    const json selection = {{"vPorigin",{.1f,-.2f,1e-6f}}, {"vRootCellSize",{8,4,2}},
        {"vSelectedCells",{root,deep}}};
    auto write = [&](const json &j) { std::ofstream out(file); out << j.dump(); assert(out.good()); };
    write(json{{"_OctreeGrid",selection}});
    const json config = {{"class","_OctreeGrid"}, {"thread",{{"FPS",30}}}, {"nP",16},
        {"nMaxLevel",40}, {"nMaxCells",100}, {"fConfig",file},
        {"vPorigin",{10,20,30}}, {"vRootCellSize",{16,16,16}}};
    Grid grid;
    grid.setName("persistedGrid");
    assert(grid.init(config)); // Load after nMaxLevel/root initialization, including depth 40.
    auto checkIDs = [&] {
        const auto ids = grid.selectedCells();
        assert(ids.size() == 2 && ids[0] == uint64_t(0));
        assert(ids[1].m_uint64[0] == 0x0123456789abcde8ULL && ids[1].m_uint64[1] == 0x2123456789abcdefULL);
    };
    checkIDs();
    OCTGRID_CELLS snapshot;
    assert(grid.get(&snapshot) == 0);
    assert(snapshot.m_header.m_vPorigin == (std::array<float,3>{.1f,-.2f,1e-6f}));
    assert(snapshot.m_header.m_vRootCellSize == (std::array<float,3>{8,4,2}));
    const json other = {{"keep",42}};
    json saved = {{"otherModule",other}};
    assert(grid.saveConfig(saved));
    assert(saved == json({{"otherModule",other}, {"_OctreeGrid",selection}}));
    json loaded;
    assert(grid.loadConfig(&loaded));
    assert(loaded == saved); checkIDs();
    assert(grid.saveConfig(saved, alternate));
    assert(!grid.saveConfig(saved, (folder / "missing" / "grid.json").string()));
    json badOutput = json::array();
    assert(!grid.saveConfig(badOutput, alternate));

    // Invalid files must preserve both the selection and the original root/occupancy.
    GEOMETRY_POINT point{vFloat3(0,0,0),vFloat4(1,1,1,1),getApproxTbootUs()};
    assert(grid.addCellPoint(point,point.m_tStamp)); grid.updateDrawAssets();
    const int count = grid.get(&snapshot);
    assert(count > 0);
    const auto header = snapshot.m_header;
    auto unchanged = [&] {
        checkIDs(); assert(grid.get(&snapshot) == count);
        assert(snapshot.m_header.m_vPorigin == header.m_vPorigin && snapshot.m_header.m_vRootCellSize == header.m_vRootCellSize);
        assert(snapshot.m_header.m_tStamp == header.m_tStamp);
    };
    vector<json> invalid;
    for (const auto &ids : vector<json>{nullptr, "bad", json::array({root,"bad"}),
        json::array({root,"40000000000000000000000000000000"}),
        json::array({root,"29000000000000000000000000000000"})}) {
        auto bad = saved;
        bad["_OctreeGrid"]["vPorigin"] = {100,200,300};
        bad["_OctreeGrid"]["vSelectedCells"] = ids;
        invalid.push_back(bad);
    }
    for (const string key : {"vPorigin", "vRootCellSize"}) {
        for (const auto &value : vector<json>{json::array({1,2}), json::array({1,2,3,4}),
            json::array({"1",2,3}), json::array({1e100,2,3}), nullptr}) {
            auto bad = saved; bad["_OctreeGrid"][key] = value; invalid.push_back(bad);
        }
    }
    auto badSize = saved; badSize["_OctreeGrid"]["vRootCellSize"] = {0,4,2}; invalid.push_back(badSize);
    auto missing = saved; missing["_OctreeGrid"].erase("vSelectedCells"); invalid.push_back(missing);
    invalid.push_back(json::object());
    for (const auto &bad : invalid) {
        write(bad);
        json output = "untouched";
        assert(!grid.loadConfig(&output) && output == "untouched"); unchanged();
    }
    { std::ofstream out(file); out << "{invalid JSON"; }
    assert(!grid.loadConfig()); unchanged();
    assert(!grid.loadConfig(nullptr,(folder / "absent.json").string())); unchanged();
    assert(grid.loadConfig(nullptr,alternate)); unchanged(); // Explicit filename overrides fConfig.

    auto moved = saved;
    moved["_OctreeGrid"]["vPorigin"] = {100,200,300};
    moved["_OctreeGrid"]["vRootCellSize"] = {4,2,1};
    write(moved); assert(grid.loadConfig()); checkIDs();
    assert(grid.get(&snapshot) == 0 && !grid.getCell(UUID128(0)));
    assert(snapshot.m_header.m_vPorigin == (std::array<float,3>{100,200,300}));
    assert(snapshot.m_header.m_vRootCellSize == (std::array<float,3>{4,2,1}));
    auto empty = moved; empty["_OctreeGrid"]["vSelectedCells"] = json::array();
    write(empty); assert(grid.loadConfig()); assert(grid.selectedCells().empty());
    json emptySaved; assert(grid.saveConfig(emptySaved)); assert(emptySaved["_OctreeGrid"] == empty["_OctreeGrid"]);
    Grid restored;
    restored.setName("restoredGrid"); assert(restored.init(config));
    assert(restored.selectedCells().empty());
    std::filesystem::remove_all(folder);
    std::cout << "PASS: selection config file round-trip, startup restore, precise IDs/root, empty lists, validation, explicit/default paths and I/O errors\n";
}

class Viewer : public _ImGUIviewer {
public:
    using _ImGUIviewer::collectGeometry;
    void buffers() { assert(m_grPt.alloc(16)); assert(m_grLn.alloc(16)); m_dTexpire = 0; }
};
int main(int argc, char **argv) {
    checkSelectionCommand();
    checkSelectionConfig();
    const int firstAlpha = argc > 1 ? int(std::stof(argv[1]) * 255 + .5f) : 64;
    const int meanAlpha = argc > 1 ? firstAlpha : 128;
    Grid grid;
    grid.setName("grid");
    assert(grid.init(json{{"class","_OctreeGrid"}, {"thread",{{"FPS",30}}}, {"nP",16},
        {"nMaxLevel",40}, {"nMaxCells",100}, {"dTexpireCell",1000},
        {"vPorigin",{10,-20,30}}, {"vRootCellSize",{8,4,2}}}));
    OCTGRID_CELLS snapshot;
    assert(grid.get(&snapshot) == 0 && snapshot.m_header.m_tStamp == 0);
    GEOMETRY_POINT point{vFloat3(13.25,-18.375,30.8125),vFloat4(1,0,0,.25),getApproxTbootUs()};
    assert(grid.addCellPoint(point, point.m_tStamp));
    for (int depth = 0; depth <= 40; ++depth) {
        auto cell = grid.getCell(point.m_vP, depth);
        assert(cell && (cell->m_ID.m_uint64[0]&63) == uint64_t(depth));
        assert(grid.getCell(cell->m_ID) == cell);
    }
    assert(!grid.getCell(UUID128(64))); // root with unused path bits
    assert(!grid.getCell(UUID128(41)));
    grid.updateDrawAssets();
    assert(grid.get(&snapshot) == 41);
    const auto stableCells = snapshot.m_vCell;
    assert(stableCells[0].id() == uint64_t(0));
    for (const auto &cell : stableCells) assert(cell.m_vC[0] == 255 && cell.m_vC[1] == 0 && cell.m_vC[2] == 0 && cell.m_vC[3] == firstAlpha);
    assert(grid.get(&snapshot,0,2) == 2);
    assert(grid.get(&snapshot,0,0) == 0 && snapshot.m_header.m_tStamp);
    assert(grid.get(&snapshot,snapshot.m_header.m_tStamp) == 0);
    point.m_vP = vFloat3(13.375,-18.25,30.875);
    point.m_vC = vFloat4(0,0,1,.75);
    grid.addCellPoint(point,getApproxTbootUs());
    for (const auto &record : stableCells) {
        auto id = record.id();
        auto cell = grid.getCell(id);
        assert(cell && cell->m_ID.m_uint64[0] == id.m_uint64[0] && cell->m_ID.m_uint64[1] == id.m_uint64[1]);
    }
    grid.updateDrawAssets();
    grid.get(&snapshot);
    assert(snapshot.m_vCell[0].m_vC[0] == 128 && snapshot.m_vCell[0].m_vC[2] == 128);
    assert(snapshot.m_vCell[0].m_vC[3] == meanAlpha);
    Viewer viewer;
    viewer.setName("viewer");
    assert(viewer.init(json{{"class","_ImGUIviewer"}, {"thread",{{"FPS",30}}},
        {"threadUI",{{"FPS",30}}}, {"nPbuf",0}, {"nLbuf",0}}));
    IMGUI_VIEWER_OBJ object;
    object.m_pGB = &grid;
    object.m_nCbuf = 2;
    viewer.collectGeometry(&grid,&object);
    assert(object.m_vBox.size() == 2 && object.m_vL.empty());
    assert(object.m_vBox[0].m_ID == uint64_t(0));
    assert(object.m_vBox[0].m_vCenter.x == 10 && object.m_vBox[0].m_vSize.y == 4);
    assert(object.m_vBox[0].m_vC.w == meanAlpha / 255.f);
    int edges = 0;
    object.m_vBox[0].forEachEdge([&](const vFloat3 &a, const vFloat3 &b) {
        assert((a.x != b.x) + (a.y != b.y) + (a.z != b.z) == 1);
        ++edges;
    });
    assert(edges == 12);
    GEOMETRY_RINGBUF<GEOMETRY_LINE> lines;
    assert(grid.get(&lines) == 0);
    grid.age(); grid.deleteExpiredCells(); grid.updateDrawAssets();
    assert(grid.get(&snapshot) == 0 && snapshot.m_vCell.empty());
    object.clearGeometry(); viewer.collectGeometry(&grid,&object);
    assert(object.m_vBox.empty() && object.m_vL.empty());
    viewer.buffers();
    _PointCloud cloud;
    _Line line;
    cloud.setName("cloud"); line.setName("line");
    assert(cloud.init(json{{"class","_PointCloud"},{"thread",{{"FPS",30}}},{"nP",16}}));
    assert(line.init(json{{"class","_Line"},{"thread",{{"FPS",30}}},{"nL",16}}));
    const vFloat3 pos(0,0,0), end(1,0,0);
    cloud.add(pos, vFloat3(1,0,0));
    cloud.add(pos, vFloat4(0,0,0,.25)); // RGB fallback must preserve alpha
    cloud.add(pos, vFloat4(1,0,0,0));
    line.add(pos, end, vFloat3(1,0,0));
    line.add(pos, end, vFloat4(0,0,0,.25));
    line.add(pos, end, vFloat4(1,0,0,0));
    object.m_matCol = vFloat4(0,1,0,.5);
    object.m_pGB = &cloud;
    viewer.collectGeometry(&cloud, &object);
    object.m_pGB = &line;
    viewer.collectGeometry(&line, &object);
    assert(object.m_vP.size() == 3 && object.m_vL.size() == 3);
    bool opaque = false, transparent = false, partial = false;
    for (const auto &p : object.m_vP) {
        opaque |= p.m_vC.w == 1; transparent |= p.m_vC.w == 0;
        partial |= p.m_vC.w == .25 && p.m_vC.y == 1;
    }
    assert(opaque && transparent && partial);
    opaque = transparent = partial = false;
    for (const auto &l : object.m_vL) {
        opaque |= l.m_vC.w == 1; transparent |= l.m_vC.w == 0;
        partial |= l.m_vC.w == .25 && l.m_vC.y == 1;
    }
    assert(opaque && transparent && partial);
    std::cout << "PASS: native ID stability/lookup at depths 0-40, RGBA, caps/expiry, ImGUI with zero point/line buffers, cells/points/lines and RGB compatibility\n";
}
