#include "../../src/Universe/Grid/_SelectableOctGrid.h"
#include "../../src/UI/Viewer/ImGUI/_ImGUIselectableOctGrid.h"
#include "../../src/UI/Viewer/Web/_WebSelectableOctGrid.h"
#include "../../src/Universe/Geometry/PointCloud/_PointCloud.h"
#include "../../src/Universe/Geometry/Line/_Line.h"
#include "../../src/Protocol/_WSconsole.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <type_traits>

using namespace kai;
static_assert(std::is_base_of_v<_ReferenceFrame, _OctreeBase>);
static_assert(!std::is_base_of_v<_GeometryBase, _OctreeBase>);
static_assert(!std::is_convertible_v<_OctreeGrid *, _GeometryBase *>);
static_assert(!std::is_convertible_v<_SelectableOctGrid *, _GeometryBase *>);
class Grid : public _SelectableOctGrid {
public:
    using _SelectableOctGrid::updateDrawAssets;
    using _SelectableOctGrid::deleteExpiredCells;
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

void checkCalculationGrid() {
    class CalculationGrid : public _OctreeGrid {
    public:
        using _OctreeGrid::deleteExpiredCells;
    } grid;
    grid.setName("calculationGrid");
    assert(grid.init(json{{"class","_OctreeGrid"}, {"thread",{{"FPS",30}}}, {"nP",1},
        {"nMaxLevel",40}, {"dTexpireCell",1}, {"vPorigin",{0,0,0}}, {"vRootCellSize",{2,2,2}}}));
    GEOMETRY_POINT point{Vector3f(.75,.5,.25), Vector3f(1,0,0),1};
    assert(grid.addCellPoint(point,1));
    for (int level = 0; level <= 40; ++level) {
        auto *cell = grid.getCell(point.m_vP,level);
        assert(cell && grid.getCell(cell->m_ID) == cell && cell->m_vC.w() == 1);
    }
    point.m_vC = Vector3f(0,0,1);
    auto *cell = grid.addCellPoint(point,1);
    assert(cell && cell->m_nP == 2 && cell->m_vC == Vector4f(.5,0,.5,1));
    assert(!grid.getCell(Vector3f(2,0,0)) && !grid.getCell(UUID128(64)));
    SelectionConsole console;
    console.source(&grid);
    console.handleJson(json{{"cmd","setGridConfig"}, {"module","calculationGrid"},
        {"vPorigin",{"100","0","0"}}, {"vRootCellSize",{"2","2","2"}}}.dump());
    assert(console.reply.is_null() && grid.getCell(point.m_vP) == cell);
    grid.deleteExpiredCells();
    assert(!grid.getCell(UUID128(0)) && !grid.getCell(point.m_vP));
    std::cout << "PASS: calculation-only grid lookup, averaging and expiry without interaction handling\n";
}

void checkGridConfigCommand() {
    Grid grid;
    grid.setName("octGrid");
    assert(grid.init(json{{"class","_SelectableOctGrid"}, {"nPminBuild",0}, {"thread",{{"FPS",30}}}, {"nP",1},
        {"nMaxLevel",4}, {"vPorigin",{0,0,0}}, {"vRootCellSize",{2,2,2}}}));
    SelectionConsole console;
    console.source(&grid);
    auto config = [](std::array<string,3> origin, std::array<string,3> size) {
        return json{{"cmd","setGridConfig"}, {"module","octGrid"}, {"vPorigin",origin}, {"vRootCellSize",size}};
    };
    const json initial = config({"0","0","0"}, {"2","2","2"});
    auto send = [&](const json &j, bool success = true) {
        console.handleJson(j.dump());
        assert(console.reply == json({{"cmd","setGridConfig"}, {"module","octGrid"}, {"bSuccess",success}}));
    };
    GEOMETRY_POINT point{Vector3f(.75,.75,.75),Vector3f(1,1,1),getApproxTbootUs()};
    auto populate = [&] { assert(grid.addCellPoint(point, point.m_tStamp)); grid.updateDrawAssets(); };
    populate();
    OCTGRID_CELLS snapshot;
    const int occupied = grid.get(&snapshot);
    const auto stamp = snapshot.m_header.m_tStamp;
    // Nested/duplicate selections must contribute volume only once during remapping.
    auto picked = initial;
    picked["cmd"] = "octGridCellSelect";
    picked["cellIDs"] = {"0200000000000000000000000000003f", "01000000000000000000000000000038", "01000000000000000000000000000038"};
    console.handleJson(picked.dump());
    assert(console.reply["bSuccess"] == true && grid.selectedCells().size() == 3);
    send(initial); // An unchanged config must preserve occupancy and exact selections.
    assert(grid.get(&snapshot) == occupied && snapshot.m_header.m_tStamp == stamp && grid.selectedCells().size() == 3);
    for (const string field : {"vPorigin", "vRootCellSize"}) {
        for (const json &value : vector<json>{nullptr, 1, "bad", json::array({"0","0"}),
             json::array({"NaN","2","2"}), json::array({"1e100","2","2"}), json::array({0,2,2})}) {
            auto bad = initial; bad[field] = value; send(bad, false);
            assert(grid.get(&snapshot) == occupied && snapshot.m_header.m_tStamp == stamp && grid.selectedCells().size() == 3);
        }
        auto missing = initial; missing.erase(field); send(missing, false);
    }
    for (const string value : {"0", "-1", "1e-100"}) {
        auto bad = initial; bad["vRootCellSize"][0] = value; send(bad, false);
    }
    const auto retainedIDs = grid.selectedCells();
    auto sameSelection = [&] {
        const auto ids = grid.selectedCells();
        return ids.size() == retainedIDs.size() && std::equal(ids.begin(), ids.end(), retainedIDs.begin(),
            [](const UUID128 &a, const UUID128 &b) {
                return a.m_uint64[0] == b.m_uint64[0] && a.m_uint64[1] == b.m_uint64[1];
            });
    };
    send(config({".25",".25",".25"}, {"2","2","2"}));
    assert(grid.get(&snapshot) == 0 && snapshot.m_header.m_tStamp > 0 && !grid.getCell(UUID128(0)));
    assert(snapshot.m_header.m_vPorigin == (std::array<float,3>{.25,.25,.25}));
    assert(sameSelection()); // The browser remaps volumes on the next snapshot.
    grid.updateDrawAssets(); assert(grid.get(&snapshot) == 0); // No stale root or publication buffer.
    populate(); assert(grid.get(&snapshot) > 0); // Rebuild from points using the new root.
    send(config({"0","0","0"}, {"4","4","4"}));
    assert(grid.get(&snapshot) == 0 && sameSelection());
    send(config({".5",".5",".5"}, {".5",".5",".5"}));
    assert(grid.get(&snapshot) == 0 && sameSelection());
    send(config({"10","-20","30"}, {"8","4","2"}));
    assert(grid.get(&snapshot) == 0 && sameSelection());
    assert(!grid.addCellPoint(point, point.m_tStamp));
    point.m_vP = Vector3f(10,-20,30); populate();
    assert(grid.get(&snapshot) > 0 && snapshot.m_header.m_vRootCellSize == (std::array<float,3>{8,4,2}));
    std::cout << "PASS: setGridConfig dispatch/ACK, atomic validation, unchanged config, occupancy reset/rebuild, selection retention for browser remapping\n";
}

// Run the production grid update loop, with a joinable thread for deterministic teardown.
class LiveGrid : public Grid {
    class Thread : public _Thread {
    public:
        std::thread worker;
        bool startThread(void *(*fn)(void *), void *arg) override {
            m_tFrom = getApproxTbootUs(); m_state = thread_run; m_bSkipSleep = false;
            run(); worker = std::thread([=] { fn(arg); }); return true;
        }
        void join() { stop(); if (worker.joinable()) worker.join(); }
    };
    _Thread *createThread(const json &j, const string &name) override {
        auto *thread = new Thread(); thread->setName(name); assert(thread->init(j)); return thread;
    }
public:
    void source(_GeometryBase *source) { m_vpGb = {source}; }
    ~LiveGrid() { if (m_pT) static_cast<Thread *>(m_pT)->join(); }
};

void checkLiveGridConfig() {
    _PointCloud cloud;
    cloud.setName("livePoints");
    assert(cloud.init(json{{"class","_PointCloud"},{"thread",{{"FPS",30}}},{"nP",1000}}));
    for (int i = 0; i < 1000; ++i) cloud.add(Vector3f(i * .001f, .5, .5), Vector3f(1,1,1));
    LiveGrid grid;
    grid.setName("liveGrid");
    assert(grid.init(json{{"class","_SelectableOctGrid"}, {"nPminBuild",0},{"thread",{{"FPS",100}}},{"nP",1000},
        {"nMaxLevel",8}, {"vPorigin",{0,0,0}}, {"vRootCellSize",{2,2,2}}}));
    grid.source(&cloud); assert(grid.start());
    SelectionConsole console; console.source(&grid);
    for (int i = 0; i < 100; ++i) {
        const string origin = i % 2 ? "100" : "0";
        console.handleJson(json{{"cmd","setGridConfig"}, {"module","liveGrid"},
            {"vPorigin",{origin,"0","0"}}, {"vRootCellSize",{"2","2","2"}}}.dump());
        assert(console.reply["bSuccess"] == true);
        OCTGRID_CELLS snapshot;
        grid.get(&snapshot);
        assert(snapshot.m_header.m_vPorigin[0] == std::stof(origin));
        if (i % 2) assert(snapshot.m_vCell.empty());
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "PASS: concurrent root changes and grid traversal publish consistent snapshots\n";
}

void checkSelectionCommand() {
    Grid grid;
    grid.setName("octGrid");
    assert(grid.init(json{{"class","_SelectableOctGrid"}, {"nPminBuild",0}, {"thread",{{"FPS",30}}}, {"nP",1},
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
    console.handleJson(json{{"cmd","loadCellSelect"}, {"module","octGrid"}}.dump());
    assert(console.reply["cmd"] == "cellSelect" && console.reply["module"] == "octGrid");
    assert(console.reply["cellIDs"] == command["cellIDs"] && console.reply["nMaxLevel"] == 40);
    for (const string key : {"vPorigin", "vRootCellSize"})
        for (int i = 0; i < 3; ++i) {
            assert(console.reply[key][i].is_string());
            assert(std::stof(console.reply[key][i].get<string>()) == std::stof(command[key][i].get<string>()));
        }
    assert(!console.reply.contains("vSelectedCells")); checkIDs();

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
    console.handleJson(json{{"cmd","loadCellSelect"}, {"module","octGrid"}}.dump());
    assert(console.reply["cmd"] == "cellSelect" && console.reply["cellIDs"].empty());
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
    write(json{{"_SelectableOctGrid",selection}});
    const json config = {{"class","_SelectableOctGrid"}, {"nPminBuild",0}, {"thread",{{"FPS",30}}}, {"nP",16},
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
    // Old section names are no longer accepted.
    write(json{{"_OctreeGrid",selection}});
    assert(!grid.loadConfig()); checkIDs();
    OCTGRID_CELLS snapshot;
    assert(grid.get(&snapshot) == 0);
    assert(snapshot.m_header.m_vPorigin == (std::array<float,3>{.1f,-.2f,1e-6f}));
    assert(snapshot.m_header.m_vRootCellSize == (std::array<float,3>{8,4,2}));
    const json other = {{"keep",42}};
    json saved = {{"otherModule",other}};
    assert(grid.saveConfig(saved));
    assert(saved.at("otherModule") == other && saved.at("_SelectableOctGrid") == selection);
    assert(saved.at("_ReferenceFrame") == json({{"vPos",{0,0,0}}, {"vOrt",{0,0,0,1}}}));
    json loaded;
    assert(grid.loadConfig(&loaded));
    assert(loaded == saved); checkIDs();
    assert(grid.saveConfig(saved, alternate));
    assert(!grid.saveConfig(saved, (folder / "missing" / "grid.json").string()));
    json badOutput = json::array();
    assert(!grid.saveConfig(badOutput, alternate));

    // Invalid files must preserve both the selection and the original root/occupancy.
    GEOMETRY_POINT point{Vector3f(0,0,0),Vector3f(1,1,1),getApproxTbootUs()};
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
        bad["_SelectableOctGrid"]["vPorigin"] = {100,200,300};
        bad["_SelectableOctGrid"]["vSelectedCells"] = ids;
        invalid.push_back(bad);
    }
    for (const string key : {"vPorigin", "vRootCellSize"}) {
        for (const auto &value : vector<json>{json::array({1,2}), json::array({1,2,3,4}),
            json::array({"1",2,3}), json::array({1e100,2,3}), nullptr}) {
            auto bad = saved; bad["_SelectableOctGrid"][key] = value; invalid.push_back(bad);
        }
    }
    auto badSize = saved; badSize["_SelectableOctGrid"]["vRootCellSize"] = {0,4,2}; invalid.push_back(badSize);
    auto missing = saved; missing["_SelectableOctGrid"].erase("vSelectedCells"); invalid.push_back(missing);
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
    moved["_SelectableOctGrid"]["vPorigin"] = {100,200,300};
    moved["_SelectableOctGrid"]["vRootCellSize"] = {4,2,1};
    write(moved); assert(grid.loadConfig()); checkIDs();
    assert(grid.get(&snapshot) == 0 && !grid.getCell(UUID128(0)));
    assert(snapshot.m_header.m_vPorigin == (std::array<float,3>{100,200,300}));
    assert(snapshot.m_header.m_vRootCellSize == (std::array<float,3>{4,2,1}));
    auto empty = moved; empty["_SelectableOctGrid"]["vSelectedCells"] = json::array();
    write(empty); assert(grid.loadConfig()); assert(grid.selectedCells().empty());
    json emptySaved; assert(grid.saveConfig(emptySaved)); assert(emptySaved["_SelectableOctGrid"] == empty["_SelectableOctGrid"]);
    Grid restored;
    restored.setName("restoredGrid"); assert(restored.init(config));
    assert(restored.selectedCells().empty());
    SelectionConsole commands;
    commands.source(&restored);
    const json picked = {{"cmd","octGridCellSelect"}, {"module","restoredGrid"},
        {"vPorigin",{"100","200","300"}}, {"vRootCellSize",{"4","2","1"}}, {"cellIDs",{root,deep}}};
    commands.handleJson(picked.dump()); // Send persists through fConfig.
    assert(commands.reply["bSuccess"] == true);
    Grid reopened;
    reopened.setName("reopenedGrid"); assert(reopened.init(config));
    assert(reopened.selectedCells().size() == 2);
    commands.source(&reopened);
    commands.handleJson(json{{"cmd","loadCellSelect"}, {"module","reopenedGrid"}}.dump());
    assert(commands.reply["cmd"] == "cellSelect" && commands.reply["module"] == "reopenedGrid");
    assert(commands.reply["cellIDs"] == picked["cellIDs"]);
    for (const string key : {"vPorigin", "vRootCellSize"})
        for (int i = 0; i < 3; ++i)
            assert(std::stof(commands.reply[key][i].get<string>()) == std::stof(picked[key][i].get<string>()));
    std::filesystem::remove_all(folder);
    std::cout << "PASS: selection config file round-trip, startup restore, precise IDs/root, empty lists, validation, explicit/default paths and I/O errors\n";
}

class Viewer : public _ImGUIselectableOctGrid {
public:
    using _ImGUIselectableOctGrid::collectGeometry;
    using _ImGUIselectableOctGrid::collectCells;
    using _ImGUIselectableOctGrid::updateAllGeometries;
    using _ImGUIselectableOctGrid::copySnapshot;
    size_t geometryCount() const { return m_sources.m_vGeometry.size(); }
    size_t gridCount() const { return m_sources.m_vGrid.size(); }
    void buffers() { assert(m_grPt.alloc(16)); assert(m_grLn.alloc(16)); m_dTexpire = 0; }
};

void checkViewerSources() {
    // ModuleMgr normally owns modules; these test sources live on the stack.
    class Sources : public ModuleMgr {
    public:
        void add(BASE *source) { m_vModules.push_back(source); }
        ~Sources() { m_vModules.clear(); }
    } sources;
    Grid grid;
    _PointCloud points;
    _ReferenceFrame frame;
    grid.setName("grid"); points.setName("points"); frame.setName("frame");
    const json thread = {{"FPS",30}};
    const json gridConfig = {{"class","_SelectableOctGrid"}, {"thread",thread}, {"nP",16}, {"nMaxLevel",1}, {"nPminBuild",0}};
    assert(grid.init(gridConfig));
    assert(points.init(json{{"class","_PointCloud"}, {"thread",thread}, {"nP",16}}));
    assert(frame.init(json{{"class","_ReferenceFrame"}, {"thread",thread}}));
    sources.add(&grid); sources.add(&points); sources.add(&frame);
    points.add(Vector3f(1,1,1),Vector3f(1,0,0));
    GEOMETRY_POINT point{Vector3f(1,1,1),Vector3f(1,0,0),getApproxTbootUs()};
    grid.addCellPoint(point,point.m_tStamp); grid.updateDrawAssets();

    auto inputConfig = gridConfig;
    inputConfig["vGeometryBase"] = {"points"};
    assert(grid.link(inputConfig,&sources));
    inputConfig["vGeometryBase"] = {"grid"};
    assert(!grid.link(inputConfig,&sources)); // A cell provider cannot be a point input.

    Viewer viewer;
    viewer.setName("viewer");
    const json viewerConfig = {{"class","_ImGUIselectableOctGrid"}, {"thread",thread}, {"threadUI",thread},
        {"nPbuf",16}, {"nLbuf",16}, {"nCbuf",16}};
    assert(viewer.init(viewerConfig));
    _WebSelectableOctGrid web;
    web.setName("web");
    auto webConfig = viewerConfig;
    webConfig["class"] = "_WebSelectableOctGrid";
    webConfig["webRoot"] = (std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() /
        "html/viewer/_SelectableOctGrid").string();
    assert(web.init(webConfig));

    const json sourceConfig = {
        {"vGeometry", {{{"_GeometryBase","points"}, {"nP",1}, {"nL",0}}}},
        {"vSelectableOctGrid", {{{"_SelectableOctGrid","grid"}, {"nC",1}}}}
    };
    auto link = [&](const json &settings, bool success) {
        auto config = viewerConfig; config.update(settings);
        assert(viewer.link(config,&sources) == success);
        config = webConfig; config.update(settings);
        assert(web.link(config,&sources) == success);
    };
    link(sourceConfig, true);
    assert(viewer.geometryCount() == 1 && viewer.gridCount() == 1);
    viewer.updateAllGeometries();
    vector<IMGUI_VIEWER_OBJ> objects;
    viewer.copySnapshot(&objects);
    assert(objects.size() == 2);
    assert(objects[0].m_name == "points" && objects[0].m_vP.size() == 1 && objects[0].m_vBox.empty());
    assert(objects[1].m_name == "grid" && objects[1].m_vP.empty() && objects[1].m_vL.empty() && objects[1].m_vBox.size() == 1);
    auto zero = sourceConfig;
    zero["vGeometry"][0]["nP"] = 0;
    zero["vSelectableOctGrid"][0]["nC"] = 0;
    link(zero, true); viewer.updateAllGeometries(); viewer.copySnapshot(&objects);
    assert(objects.empty());
    auto hidden = sourceConfig;
    hidden["vGeometry"][0]["bVisible"] = false;
    hidden["vSelectableOctGrid"][0]["bVisible"] = false;
    link(hidden, true); viewer.updateAllGeometries(); viewer.copySnapshot(&objects);
    assert(objects.empty());
    for (const json &invalid : vector<json>{
        {{"vReferenceFrame",{"grid","points"}}}, {{"vGeometryBase",{"points"}}},
        {{"vGeometry", {{{"_ReferenceFrame","grid"}}}}},
        {{"vGeometry", {{{"_GeometryBase","grid"}}}}},
        {{"vSelectableOctGrid", {{{"_SelectableOctGrid","points"}}}}},
        {{"vSelectableOctGrid", {{{"_SelectableOctGrid","frame"}}}}},
        {{"vGeometry", {{{"_GeometryBase","points"}}, {{"_GeometryBase","points"}}}}},
        {{"vGeometry", {{{"_GeometryBase","points"}, {"nPbuf",1}}}}},
        {{"vGeometry", {{{"_GeometryBase","points"}, {"nP",-1}}}}},
        {{"vSelectableOctGrid", {{{"_SelectableOctGrid","grid"}, {"nC",-1}}}}},
        {{"vSelectableOctGrid", {{{"_SelectableOctGrid","grid"}, {"nP",1}}}}},
        {{"vGeometry", {{{"_GeometryBase","missing"}}}}},
        {{"vGeometry", {"points"}}}
    }) link(invalid, false);
    auto obsolete = gridConfig; obsolete["nMaxLines"] = 12;
    Grid oldGrid; oldGrid.setName("oldGrid"); assert(!oldGrid.init(obsolete));
    std::cout << "PASS: separate typed sources, caps/visibility, and rejection of wrong providers, duplicates and obsolete configuration\n";

}

int main(int argc, char **argv) {
    checkViewerSources();
    checkCalculationGrid();
    checkGridConfigCommand();
    checkLiveGridConfig();
    checkSelectionCommand();
    checkSelectionConfig();
    const int firstAlpha = argc > 1 ? int(std::stof(argv[1]) * 255 + .5f) : 255;
    const int meanAlpha = firstAlpha;
    Grid grid;
    grid.setName("grid");
    assert(grid.init(json{{"class","_SelectableOctGrid"}, {"nPminBuild",0}, {"thread",{{"FPS",30}}}, {"nP",16},
        {"nMaxLevel",40}, {"nMaxCells",100}, {"dTexpireCell",1000},
        {"vPorigin",{10,-20,30}}, {"vRootCellSize",{8,4,2}}}));
    OCTGRID_CELLS snapshot;
    assert(grid.get(&snapshot) == 0 && snapshot.m_header.m_tStamp == 0);
    GEOMETRY_POINT point{Vector3f(13.25,-18.375,30.8125),Vector3f(1,0,0),getApproxTbootUs()};
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
    point.m_vP = Vector3f(13.375,-18.25,30.875);
    point.m_vC = Vector3f(0,0,1);
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
    assert(viewer.init(json{{"class","_ImGUIselectableOctGrid"}, {"thread",{{"FPS",30}}},
        {"threadUI",{{"FPS",30}}}, {"nPbuf",0}, {"nLbuf",0}}));
    IMGUI_VIEWER_OBJ object;
    VIEWER_GRID_SOURCE cellSource;
    cellSource.m_pGrid = &grid;
    cellSource.m_nC = 2;
    viewer.collectCells(cellSource,&object,0);
    assert(object.m_vBox.size() == 2 && object.m_vL.empty());
    assert(object.m_vBox[0].m_ID == uint64_t(0));
    assert(object.m_vBox[0].m_vCenter.x() == 10 && object.m_vBox[0].m_vSize.y() == 4);
    assert(object.m_vBox[0].m_vC.w() == meanAlpha / 255.f);
    int edges = 0;
    object.m_vBox[0].forEachEdge([&](const Vector3f &a, const Vector3f &b) {
        assert((a.x() != b.x()) + (a.y() != b.y()) + (a.z() != b.z()) == 1);
        ++edges;
    });
    assert(edges == 12);

    grid.age(); grid.deleteExpiredCells(); grid.updateDrawAssets();
    assert(grid.get(&snapshot) == 0 && snapshot.m_vCell.empty());
    object.clearGeometry(); viewer.collectCells(cellSource,&object,0);
    assert(object.m_vBox.empty() && object.m_vL.empty());
    viewer.buffers();
    object.clearGeometry(); viewer.collectCells(cellSource,&object,0);
    assert(object.m_vP.empty() && object.m_vL.empty() && object.m_vBox.empty());
    _PointCloud cloud;
    _Line line;
    cloud.setName("cloud"); line.setName("line");
    assert(cloud.init(json{{"class","_PointCloud"},{"thread",{{"FPS",30}}},{"nP",16}}));
    assert(line.init(json{{"class","_Line"},{"thread",{{"FPS",30}}},{"nL",16}}));
    const Vector3f pos(0,0,0), end(1,0,0);
    cloud.add(pos, Vector3f(1,0,0));
    cloud.add(pos, Vector3f(0,0,0)); // Black uses the material RGB fallback
    cloud.add(pos, Vector3f(0,0,1));
    line.add(pos, end, Vector3f(1,0,0));
    line.add(pos, end, Vector3f(0,0,0));
    line.add(pos, end, Vector3f(0,0,1));
    VIEWER_GEOMETRY_SOURCE geometrySource;
    geometrySource.m_matCol = Vector4f(0,1,0,1);
    geometrySource.m_nP = geometrySource.m_nL = 16;
    geometrySource.m_pGeometry = &cloud;
    viewer.collectGeometry(geometrySource, &object,0);
    geometrySource.m_pGeometry = &line;
    viewer.collectGeometry(geometrySource, &object,0);
    assert(object.m_vP.size() == 3 && object.m_vL.size() == 3);
    auto checkRGB = [](const auto &vertices) {
        bool red = false, green = false, blue = false;
        for (const auto &v : vertices) {
            static_assert(decltype(v.m_vC)::SizeAtCompileTime == 3);
            red |= v.m_vC == Vector3f(1,0,0);
            green |= v.m_vC == Vector3f(0,1,0);
            blue |= v.m_vC == Vector3f(0,0,1);
        }
        assert(red && green && blue);
    };
    checkRGB(object.m_vP);
    checkRGB(object.m_vL);
    std::cout << "PASS: native ID stability/lookup at depths 0-40, RGBA, caps/expiry, ImGUI with zero point/line buffers, cells/points/lines and RGB compatibility\n";
}
