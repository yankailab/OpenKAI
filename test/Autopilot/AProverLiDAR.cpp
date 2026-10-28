#include "../../samples/AProver_LiDAR/_AProver_LiDAR.h"
#include <cassert>
#include <cmath>
#include <iostream>
#if defined(USE_SCEPTER_SDK) && defined(USE_WSSERVER) && defined(WITH_NET)
#include "../../src/Vision/RGBD/_Scepter.h"
#endif

namespace
{

using namespace kai;

class GridHarness : public _SelectableOctGrid
{
public:
    void add(unsigned count, uint64_t stamp = getTns())
    {
        std::lock_guard<std::mutex> lock(m_gridMutex);
        GEOMETRY_POINT point;
        point.m_vP = {0, 0, 0};
        for (unsigned i = 0; i < count; ++i)
            addCellPoint(point, stamp);
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_gridMutex);
        m_pCell->release();
    }
};

class AutopilotHarness : public _APmav_rover
{
public:
    void attach(MavlinkStream &stream)
    {
        m_pMavStream = &stream;
        m_customModeFC = AP_ROVER_MANUAL;
        m_modeFC = apMode_manual;
        m_armFC = apArm_arm;
    }
    void report(int mode, AP_ARM arm) { m_customModeFC = mode; m_armFC = arm; }
    int requestedMode() const { return m_customMode; }
    AP_ARM requestedArm() const { return m_arm; }
};

class RoverHarness : public _AProver_LiDAR
{
public:
    using _AProver_LiDAR::updateCtrl;
    using _AProver_LiDAR::updateDrive;
    using _AProver_LiDAR::onPause;

    void attach(AutopilotHarness &autopilot, GridHarness &grid, _GeoFence &fence)
    {
        m_pAP = &autopilot;
        m_pOctGrid = &grid;
        m_pGfence = &fence;
    }

    void command(const json &command) { console(command, nullptr); }
    float steer() const { return m_steer; }
    float speed() const { return m_speed; }
    AP_LIDAR_MODE mode() const { return m_dMode; }
    bool cutter() const { return m_bServoON; }
    void expireManual() { m_tLastBtn = getTns() - m_tOutBtn - 1; }
};

struct Fixture
{
    JsonCfg cfg;
    MavlinkStream stream;
    AutopilotHarness autopilot;
    GridHarness grid;
    _GeoFence fence;
    RoverHarness rover;

    explicit Fixture(double turn = 60)
    {
        cfg.setJson({
            {"stream", {{"class", "MavlinkStream"}}},
            {"autopilot", {{"class", "_APmav_rover"}}},
            {"grid", {{"class", "_SelectableOctGrid"}, {"nMaxLevel", 1}, {"nMaxCells", 0}, {"dTexpireCell", NSEC_SEC},
                {"vSelectedCells", {"00000000000000000000000000000000"}}}},
            {"fence", {{"class", "_GeoFence"}, {"type", _GeoFence_polygon}, {"estD", 2},
                {"vPolygon", {{-0.001, -0.001}, {-0.001, 0.001}, {0.001, 0.001}, {0.001, -0.001}}}}},
            {"rover", {{"class", "_AProver_LiDAR"}, {"bumperTurnDeg", turn}, {"hdgToleranceDeg", 5}, {"tOutBtn", NSEC_SEC},
                {"speedGo", 0.3}, {"steerTurn", 0.4}, {"octGridOccu", 2}}},
        });
        auto &config = *cfg.getJson();
        auto load = [&](BASE &object, const char *name)
        {
            object.setName(name);
            object.setConfig(&cfg, &config[name]);
            assert(object.loadConfig());
        };
        load(stream, "stream");
        load(autopilot, "autopilot");
        load(grid, "grid");
        load(fence, "fence");
        load(rover, "rover");
        autopilot.attach(stream);
        rover.attach(autopilot, grid, fence);
        position(0);
        buttons(0);
    }

    void position(double heading, double lat = 0, double lon = 0)
    {
        mavlink_global_position_int_t position{};
        position.lat = std::lround(lat * 1e7);
        position.lon = std::lround(lon * 1e7);
        position.hdg = std::lround(heading * 100);
        mavlink_message_t message{};
        mavlink_msg_global_position_int_encode(1, 1, &message, &position);
        assert(stream.decode(message));
    }

    void buttons(uint8_t state)
    {
        mavlink_button_change_t button{};
        button.state = state;
        mavlink_message_t message{};
        mavlink_msg_button_change_encode(1, 1, &message, &button);
        assert(stream.decode(message));
    }

    void start()
    {
        rover.command({{"cmd", "startAuto"}});
        assert(rover.updateCtrl());
        assert(rover.mode() == apLiDAR_modeAuto);
    }
};

void testSelectedOccupancy()
{
    Fixture f;
    assert(!f.grid.bSelectedOccupied(2));
    f.grid.add(2);
    assert(!f.grid.bSelectedOccupied(2)); // Strictly over the configured threshold.
    f.grid.add(1);
    assert(f.grid.bSelectedOccupied(2)); // Independent of the viewer's zero cell cap.
    f.grid.clear();
    f.grid.add(3, getTns() - 2 * NSEC_SEC);
    assert(!f.grid.bSelectedOccupied(2)); // Ignore stale cells before the next grid sweep.
}

void testManualAndStop()
{
    Fixture f;
    for (const char *button : {"F", "B", "L", "R"})
    {
        f.rover.command({{"cmd", "ctrlBtn"}, {"btn", button}});
        assert(f.rover.updateCtrl());
        assert(f.rover.mode() == apLiDAR_modeManual);
        if (*button == 'F') assert(f.rover.speed() > 0 && f.rover.steer() == 0);
        if (*button == 'B') assert(f.rover.speed() < 0 && f.rover.steer() == 0);
        if (*button == 'L') assert(f.rover.speed() == 0 && f.rover.steer() < 0);
        if (*button == 'R') assert(f.rover.speed() == 0 && f.rover.steer() > 0);
    }
    f.rover.expireManual();
    assert(f.rover.updateCtrl());
    assert(f.rover.mode() == apLiDAR_modeStandby && f.rover.speed() == 0 && f.rover.steer() == 0);
    f.start();
    assert(f.rover.speed() > 0);
    f.rover.command({{"cmd", "ctrlBtn"}, {"btn", "S"}});
    assert(f.rover.mode() == apLiDAR_modeStandby && f.rover.speed() == 0 && f.rover.steer() == 0);
    assert(f.rover.updateCtrl());
    assert(f.rover.mode() == apLiDAR_modeStandby && f.rover.speed() == 0 && f.rover.steer() == 0);
    f.start(); // STOP's old button state must not override an explicit Auto request.
    assert(f.rover.speed() > 0);
}

void testObstacle()
{
    Fixture f;
    f.start();
    assert(f.rover.speed() > 0);
    f.grid.add(3);
    assert(f.rover.updateCtrl());
    const float steer = f.rover.steer();
    assert(steer != 0 && f.rover.speed() == 0);
    assert(f.rover.updateCtrl());
    assert(f.rover.steer() == steer && f.rover.speed() == 0);
    f.grid.clear();
    assert(f.rover.updateCtrl());
    assert(f.rover.speed() > 0 && f.rover.steer() == 0);
}

void testEmergency()
{
    Fixture f;
    f.start();
    f.rover.command({{"cmd", "setServo"}, {"bServoON", true}});
    assert(f.rover.cutter());
    f.buttons(1);
    assert(f.rover.updateCtrl());
    assert(f.rover.mode() == apLiDAR_modeStandby && !f.rover.cutter());
    assert(f.rover.speed() == 0 && f.rover.steer() == 0);
    f.rover.command({{"cmd", "startAuto"}});
    f.rover.command({{"cmd", "setServo"}, {"bServoON", true}});
    assert(f.rover.updateCtrl());
    assert(f.rover.mode() == apLiDAR_modeStandby && !f.rover.cutter());
    f.rover.updateDrive();
    vector<mavlink_message_t> messages;
    f.stream.getEncodedMsgs(messages);
    assert(!messages.empty());
    mavlink_rc_channels_override_t rc{};
    mavlink_msg_rc_channels_override_decode(&messages.back(), &rc);
    assert(rc.chan1_raw == 1500 && rc.chan3_raw == 1500 && rc.chan6_raw == 1500);
    f.buttons(0);
    assert(f.rover.updateCtrl());
    assert(f.rover.mode() == apLiDAR_modeStandby && !f.rover.cutter());
}

void testBumper()
{
    for (const double offset : {60., -60., 180., -180.})
    {
        Fixture f(offset);
        f.position(350);
        f.start();
        f.buttons(2);
        assert(f.rover.updateCtrl());
        assert(f.rover.speed() < 0 && f.rover.steer() == 0);
        assert(f.rover.updateCtrl());
        assert(f.rover.speed() < 0 && f.rover.steer() == 0);
        f.buttons(0);
        assert(f.rover.updateCtrl());
        assert(f.rover.speed() == 0 && f.rover.steer() * offset > 0);
        const double target = std::fmod(350 + offset + 360, 360);
        f.position(target);
        assert(f.rover.updateCtrl());
        assert(f.rover.speed() > 0 && f.rover.steer() == 0);
    }

    Fixture f;
    f.start();
    f.buttons(2);
    assert(f.rover.updateCtrl());
    f.buttons(0);
    assert(f.rover.updateCtrl());
    assert(f.rover.steer() > 0 && f.rover.speed() == 0);
    f.position(20);
    f.buttons(2); // A second contact cancels the turn and backs away again.
    assert(f.rover.updateCtrl());
    assert(f.rover.speed() < 0 && f.rover.steer() == 0);
    f.buttons(0);
    assert(f.rover.updateCtrl());
    assert(f.rover.speed() == 0 && f.rover.steer() > 0);
    f.position(80);
    assert(f.rover.updateCtrl());
    assert(f.rover.speed() > 0 && f.rover.steer() == 0);
}

void testModeAndArmingGate()
{
    Fixture f;
    f.autopilot.report(AP_ROVER_HOLD, apArm_disarm);
    f.rover.command({{"cmd", "ctrlBtn"}, {"btn", "F"}});
    assert(f.rover.updateCtrl());
    assert(f.rover.speed() == 0 && f.rover.steer() == 0);
    assert(f.autopilot.requestedMode() == AP_ROVER_MANUAL);
    f.autopilot.report(AP_ROVER_MANUAL, apArm_disarm);
    assert(f.rover.updateCtrl());
    assert(f.rover.speed() == 0 && f.autopilot.requestedArm() == apArm_arm);
    f.autopilot.report(AP_ROVER_MANUAL, apArm_arm);
    assert(f.rover.updateCtrl());
    assert(f.rover.speed() > 0);
    f.rover.command({{"cmd", "setServo"}, {"bServoON", true}});
    f.rover.onPause();
    assert(f.rover.mode() == apLiDAR_modeStandby && !f.rover.cutter());
    assert(f.rover.speed() == 0 && f.rover.steer() == 0);

    f.autopilot.report(AP_ROVER_HOLD, apArm_disarm);
    f.rover.command({{"cmd", "setSteerSpeed"}, {"steer", -0.2}, {"speed", 0.5}});
    assert(f.rover.updateCtrl());
    assert(f.rover.speed() == 0 && f.rover.steer() == 0);
    f.autopilot.report(AP_ROVER_MANUAL, apArm_arm);
    assert(f.rover.updateCtrl());
    assert(std::fabs(f.rover.speed() - 0.5) < 1e-6 && std::fabs(f.rover.steer() + 0.2) < 1e-6);
}

void testWorkerStop()
{
    Fixture f;
    f.start();
    f.rover.command({{"cmd", "setServo"}, {"bServoON", true}});
    assert(f.rover.start());
    f.rover.stop(); // Joins before checking state and the final transmitted override.
    assert(f.rover.mode() == apLiDAR_modeStandby && !f.rover.cutter());
    assert(f.rover.speed() == 0 && f.rover.steer() == 0);
    vector<mavlink_message_t> messages;
    f.stream.getEncodedMsgs(messages);
    assert(!messages.empty());
    mavlink_rc_channels_override_t rc{};
    mavlink_msg_rc_channels_override_decode(&messages.back(), &rc);
    assert(rc.chan1_raw == 1500 && rc.chan3_raw == 1500 && rc.chan6_raw == 1500);
}

void testFenceTurns()
{
    // North boundary; NE must reflect towards SE, NW towards SW.
    for (double heading : {45., 315.})
    {
        Fixture f;
        f.position(heading, 0.000995, 0);
        f.start();
        assert(f.rover.speed() == 0);
        if (heading == 45) assert(f.rover.steer() > 0);
        else assert(f.rover.steer() < 0);
        f.position(heading == 45 ? 135 : 225, 0.000995, 0);
        assert(f.rover.updateCtrl());
        assert(f.rover.speed() > 0 && f.rover.steer() == 0);
    }

    Fixture f;
    f.position(315, 0.000995, 0);
    f.grid.add(3);
    f.start();
    assert(f.rover.speed() == 0 && f.rover.steer() < 0);
    f.position(270, 0.000995, 0);
    assert(f.rover.updateCtrl());
    // Keep the fence-selected direction while LiDAR still blocks the path.
    assert(f.rover.speed() == 0 && f.rover.steer() < 0);
    f.grid.clear();
    assert(f.rover.updateCtrl());
    assert(f.rover.speed() > 0 && f.rover.steer() == 0);
}

void testInvalidNavigationStopsAuto()
{
    Fixture f;
    f.start();
    f.rover.command({{"cmd", "setServo"}, {"bServoON", true}});
    // 65535 is MAVLink's unknown heading marker, not a valid bearing.
    f.position(655.35);
    assert(f.rover.updateCtrl());
    assert(f.rover.mode() == apLiDAR_modeStandby && !f.rover.cutter());
    assert(f.rover.speed() == 0 && f.rover.steer() == 0);

    f.position(0);
    f.start();
    f.stream.get<MavGlobalPositionINT>()->updateTstamp(getTns() - 3 * NSEC_SEC);
    assert(f.rover.updateCtrl());
    assert(f.rover.mode() == apLiDAR_modeStandby && f.rover.speed() == 0);

    f.position(0);
    f.fence.setPolygon({});
    f.rover.command({{"cmd", "startAuto"}});
    assert(f.rover.updateCtrl());
    assert(f.rover.mode() == apLiDAR_modeStandby && f.rover.speed() == 0);
}

void testSampleConfigPersistence(const string &path)
{
#if defined(USE_SCEPTER_SDK) && defined(USE_WSSERVER) && defined(WITH_NET)
    JsonCfg source;
    assert(source.readFromFile("samples/AProver_LiDAR/AProver_LiDAR.json"));
    // Console initialization alone enters curses; all application modules remain enabled.
    (*source.getJson())["console"]["bON"] = false;
    assert(source.saveToFile(path));
    auto load = [&](InstanceMgr &instances)
    {
        assert(instances.loadJsonFiles(path));
        assert(instances.createAll());
        assert(instances.addExternalModule(new _AProver_LiDAR(), "apRover"));
        assert(instances.initAll());
        assert(instances.linkAll());
        for (const char *name : {"apRover", "scepter", "octGrid", "geoFence", "viewer", "scepterViewer", "wsConsole"})
            assert(instances.findModule(name));
        // No startAll(): this test must neither connect devices nor open network listeners.
    };

    const string rootID = "00000000000000000000000000000000";
    {
        InstanceMgr instances;
        load(instances);
        auto *grid = static_cast<_SelectableOctGrid *>(instances.findModule("octGrid"));
        auto *scepter = static_cast<_Scepter *>(instances.findModule("scepter"));
        const json before = *instances.findJsonCfg("apRover")->getJson();
        json selection = {{"cmd", "octGridCellSelect"}, {"cellIDs", {rootID}}};
        for (const char *key : {"vPorigin", "vRootCellSize"})
        {
            selection[key] = json::array();
            for (const auto &coordinate : before["octGrid"][key])
                selection[key].push_back(coordinate.dump());
        }
        grid->console(selection, nullptr); // Same native handler used by the grid viewer.
        auto controls = scepter->getCamCtrl();
        controls.m_confidenceFilter.threshold = 17;
        assert(scepter->setCamCtrl(controls)); // Applies cached controls while the device is closed.
        assert(scepter->saveConfig(true));

        JsonCfg saved;
        assert(saved.readFromFile(path));
        const json &after = *saved.getJson();
        assert(after["octGrid"]["vSelectedCells"] == json::array({rootID}));
        assert(after["scepter"]["scConfidenceFilterThreshold"] == 17);
        for (auto it = before.begin(); it != before.end(); ++it)
            if (it.key() != "scepter" && it.key() != "octGrid")
                assert(after.at(it.key()) == it.value());
    }
    {
        InstanceMgr reloaded;
        load(reloaded);
        auto *grid = static_cast<_SelectableOctGrid *>(reloaded.findModule("octGrid"));
        auto *scepter = static_cast<_Scepter *>(reloaded.findModule("scepter"));
        assert(grid->getSelectedCells().size() == 1);
        assert(scepter->getCamCtrl().m_confidenceFilter.threshold == 17);
    }
#else
    std::cout << "Sample configuration persistence requires Scepter, WebSocket, and HTTP modules\n";
#endif
}

}

int main(int argc, char **argv)
{
    assert(argc == 2);
    testSelectedOccupancy();
    testManualAndStop();
    testObstacle();
    testEmergency();
    testBumper();
    testModeAndArmingGate();
    testWorkerStop();
    testFenceTurns();
    testInvalidNavigationStopsAuto();
    testSampleConfigPersistence(argv[1]);
    std::cout << "AProver LiDAR regressions passed\n";
}
