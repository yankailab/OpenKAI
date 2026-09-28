#include "../../src/Base/_ModuleBase.h"
#include "../../src/Protocol/_JSONbase.h"
#include "../../src/UI/_Console.h"
#include <cassert>
#include <limits>
// Expose receiver callbacks and state only inside this standalone fixture.
#define private public
#define protected public
#include "../../src/Universe/Grid/_OctreeGrid.h"
#include "../../src/Sensor/LiDAR/_Livox2.h"
#undef private
#undef protected
using namespace kai;
// The fixture isolates these modules from the full application registry and UI.
std::map<string, DataStreamBase *> registry;
namespace kai
{
void _Console::addMsg(const string &, int)
{
}
void _Console::addMsg(const string &, int, int, int)
{
}
InstanceMgr::InstanceMgr() = default;
InstanceMgr::~InstanceMgr() = default;
void *InstanceMgr::findDataStream(const string &name)
{
    auto it = registry.find(name);
    return it == registry.end() ? nullptr : it->second;
}
void *InstanceMgr::findModule(const string &)
{
    return nullptr;
}
}

class TestGrid : public _OctreeGrid
{
public:
    bool check(void) override
    {
        return true;
    }

    TestGrid()
    {
        m_pCell = new OCTREE_CELL<OCTGRID_PCL_CELL>();
        m_nMaxLevel = 0;
    }
};

GEOMETRY_POINT point(uint64_t stamp)
{
    GEOMETRY_POINT result;
    result.m_tStamp = stamp;
    result.m_vC = {1, 0, 0};
    return result;
}

void testGrid()
{
    PCLframe input;
    TestGrid grid;
    grid.m_vPointInputs.push_back({&input, 0});
    grid.updatePoint();
    assert(!grid.m_pCell->getT());
    GEOMETRY_POINT invalid = point(1);
    invalid.m_vP.x() = std::numeric_limits<float>::quiet_NaN();
    input.set({point(1), point(0), invalid});
    grid.updatePoint();
    assert(grid.m_pCell->getT()->m_nP == 1);
    grid.updatePoint();
    assert(grid.m_pCell->getT()->m_nP == 1);
    input.set({point(1)});
    grid.updatePoint();
    assert(grid.m_pCell->getT()->m_nP == 2);
    grid.m_pCell->release();
    grid.resetPointInputs();
    grid.updatePoint();
    assert(grid.m_pCell->getT()->m_nP == 1);
    input.set({});
    grid.updatePoint();
    assert(grid.m_pCell->getT()->m_nP == 1);
    grid.m_dTexpirePCL = 1;
    input.set({point(1)});
    grid.updatePoint();
    assert(grid.m_pCell->getT()->m_nP == 1);
    grid.m_dTexpireCell = 1;
    grid.m_pCell->getT()->m_tStamp = 1;
    grid.deleteExpiredCells();
    assert(!grid.m_pCell->getT());
    grid.m_dTexpirePCL = 0;
    input.set(vector<GEOMETRY_POINT>(100005, point(1)));
    grid.updatePoint();
    assert(grid.m_pCell->getT()->m_nP == 100005);
    std::cout << "PASS grid: revision deduplication, root reset replay, invalid/expired points, cell expiry, no ring truncation\n";
}

LIVOX2_DATA packet(uint8_t frame, uint64_t stamp, uint16_t count = 1)
{
    LIVOX2_DATA data{};
    data.frame_cnt = frame;
    data.data_type = kLivoxLidarCartesianCoordinateHighData;
    data.time_interval = 10;
    data.dot_num = count;
    data.length = 36 + count * sizeof(LivoxLidarCartesianHighRawPoint);
    memcpy(data.timestamp, &stamp, sizeof(stamp));
    LivoxLidarCartesianHighRawPoint raw{};
    raw.x = 1000;
    raw.y = 2000;
    raw.z = 3000;
    for (size_t i = 0; i < count; ++i)
    {
        memcpy(data.data + i * sizeof(raw), &raw, sizeof(raw));
    }
    return data;
}

void testLivox()
{
    PCLframe output;
    _Livox2 lidar;
    lidar.m_pPCL = &output;
    lidar.handlePointCloudData(packet(255, 100, 2));
    lidar.handlePointCloudData(packet(255, 120));
    assert(output.get()->m_revision == 0);
    lidar.handlePointCloudData(packet(0, 200));
    auto frame = output.get();
    assert(frame->m_vPoints.size() == 3 && frame->m_tStamp == 100);
    assert(frame->m_vPoints[0].m_vP.isApprox(Vector3f(1, 2, 3)));
    assert(frame->m_vPoints[1].m_tStamp == 600);
    lidar.clear();
    assert(output.get()->m_vPoints.empty());
    lidar.handlePointCloudData(packet(1, 300));
    lidar.handlePointCloudData(packet(2, 400));
    assert(output.get()->m_vPoints.size() == 1 && output.get()->m_tStamp == 300);
    lidar.clear();
    lidar.m_nMaxFramePoints = 2;
    lidar.handlePointCloudData(packet(3, 500, 3));
    lidar.handlePointCloudData(packet(4, 600));
    assert(output.get()->m_vPoints.size() == 2);
    const uint64_t revision = output.get()->m_revision;
    auto invalid = packet(5, 700);
    invalid.dot_num = 0;
    lidar.handlePointCloudData(invalid);
    invalid = packet(5, 700);
    invalid.length = 36;
    lidar.handlePointCloudData(invalid);
    invalid = packet(5, 700);
    invalid.dot_num = 1000;
    lidar.handlePointCloudData(invalid);
    assert(output.get()->m_revision == revision);
    assert(frame->m_vPoints.size() == 3 && frame->m_tStamp == 100);
    IMUstream imu;
    lidar.m_pIMU = &imu;
    LIVOX2_DATA data{};
    data.length = 36 + sizeof(LivoxLidarImuRawPoint);
    LivoxLidarImuRawPoint raw{};
    raw.acc_x = 1;
    raw.gyro_y = 2;
    uint64_t stamp = 800;
    memcpy(data.timestamp, &stamp, sizeof(stamp));
    memcpy(data.data, &raw, sizeof(raw));
    lidar.handleIMUdata(data);
    const auto imuFrame = imu.get();
    const auto &gyro = imuFrame->m_dqGyro;
    const auto &acc = imuFrame->m_dqAcc;
    assert(gyro.size() == 1 && gyro[0].m_t == 800 && gyro[0].m_v.y() == 2);
    assert(acc.size() == 1 && acc[0].m_t == 800 && acc[0].m_v.x() == 1);
    std::cout << "PASS Livox: frame boundaries/wrap, retained snapshots, pose/unit conversion, bounded accumulation, clear discards pending frame, malformed packets, IMUstream output\n";
}

int main()
{
    testGrid();
    testLivox();
}
