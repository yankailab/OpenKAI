#ifdef OPENKAI_LIVOX2_TEST
#include "../../src/Sensor/LiDAR/_Livox2.h"
#include <cassert>
#include <iostream>

namespace kai
{
    struct Livox2TestAccess
    {
        static void outputs(_Livox2 &driver, PCLframe &cloud, IMUstream &imu, BytePacketStream &commands)
        { driver.m_pPCLout = &cloud; driver.m_pIMUout = &imu; driver.m_pBpStreamCtrlCmdOut = &commands; }
        static bool command(_Livox2 &driver, const vector<uint8_t> &bytes, LIVOX2_CMD &result)
        { return driver.recvLivoxCmd({bytes, 1}, &result); }
        static bool data(_Livox2 &driver, const vector<uint8_t> &bytes, LIVOX2_DATA &result)
        { return driver.recvLivoxData({bytes, 1}, &result); }
        static void points(_Livox2 &driver, const LIVOX2_DATA &packet) { driver.handlePointCloudData(packet); }
        static void imu(_Livox2 &driver, const LIVOX2_DATA &packet) { driver.handleIMUdata(packet); }
        static json snapshot(_Livox2 &driver) { return driver.imuValues(); }
        static bool config(_Livox2 &driver, const json &patch, json &errors) { return driver.applyConfig(patch, true, errors); }
        static json values(_Livox2 &driver) { return driver.configValues(); }
        static void queue(_Livox2 &driver, uint16_t key, uint8_t value) { driver.setLvxByte(key, value); }
        static size_t pending(_Livox2 &driver) { return driver.m_pending.size(); }
        static uint32_t sequence(_Livox2 &driver) { return driver.m_sequence; }
        static bool failed(_Livox2 &driver) { return driver.m_configFailed; }
        static void connected(_Livox2 &driver) { driver.m_lvxState = lvxState_work; }
        static vector<LIVOX2_CMD> pendingCommands(_Livox2 &driver)
        {
            vector<LIVOX2_CMD> result;
            for (const auto &pending : driver.m_pending) result.push_back(pending.second.command);
            return result;
        }
        static void ack(_Livox2 &driver, const LIVOX2_CMD &reply) { driver.handleCtrlCmdAck(reply); }
        static void info(_Livox2 &driver, const vector<uint8_t> &tlv, uint16_t count) { driver.handleInfo(tlv.data(), tlv.size(), count); }
        static json observed(_Livox2 &driver) { return driver.m_observed; }
    };
}
namespace
{
    using namespace kai;
    void put(vector<uint8_t> &data, size_t offset, uint64_t value, size_t size)
    { for (size_t i = 0; i < size; ++i) data[offset + i] = uint8_t(value >> (8 * i)); }

    vector<uint8_t> packet(uint8_t type, uint16_t count, uint64_t stamp, bool withCRC = true)
    {
        vector<uint8_t> bytes(36 + livox2::pointSize(type) * count);
        put(bytes, 1, bytes.size(), 2); put(bytes, 3, 100, 2); put(bytes, 5, count, 2);
        bytes[10] = type; put(bytes, 28, stamp, 8);
        if (withCRC) put(bytes, 24, CRC::Calculate(bytes.data() + 28, bytes.size() - 28, CRC::CRC_32()), 4);
        return bytes;
    }

    void testProtocol()
    {
        _Livox2 driver; LIVOX2_DATA data; LIVOX2_CMD command;
        auto bytes = packet(1, 96, 1000000000);
        assert(Livox2TestAccess::data(driver, bytes, data));
        for (size_t size = 0; size < bytes.size(); ++size)
            assert(!livox2::validData(bytes.data(), size));
        auto corrupt = bytes; corrupt.back() ^= 1; assert(!livox2::validData(corrupt.data(), corrupt.size()));
        put(corrupt, 24, 0, 4); assert(livox2::validData(corrupt.data(), corrupt.size()));
        corrupt[10] = 4; assert(!livox2::validData(corrupt.data(), corrupt.size()));
        corrupt = bytes; put(corrupt, 5, 65535, 2); assert(!livox2::validData(corrupt.data(), corrupt.size()));
        corrupt = bytes; put(corrupt, 28, UINT64_MAX, 8); assert(!livox2::validData(corrupt.data(), corrupt.size(), false));
        auto imu = packet(0, 1, 1000000000);
        assert(livox2::validData(imu.data(), imu.size()));
        put(imu, 24, 0, 4); assert(!livox2::validData(imu.data(), imu.size()));
        assert(livox2::pointTime(100, 4750, 96, 95) == 475100);
        assert(livox2::pointTime(100, 4750, 96, 1) == 5100);
        assert(livox2::pointTime(100, 4750, 1, 0) == 100);

        command.init(LVX2_CMD_GET, LVX2_CMD_ACK, 42); command.sender_type = 1;
        command.addData(uint8_t(0)); command.addData(uint16_t(0)); command.calcCRC();
        bytes.assign(reinterpret_cast<uint8_t *>(&command), reinterpret_cast<uint8_t *>(&command) + command.length);
        LIVOX2_CMD received;
        assert(Livox2TestAccess::command(driver, bytes, received)); assert(received.seq_num == 42);
        for (size_t size = 0; size < bytes.size(); ++size) assert(!livox2::validCommand(bytes.data(), size));
        bytes[12] ^= 1; assert(!Livox2TestAccess::command(driver, bytes, received));
        command.init(LVX2_CMD_DISCOVER, LVX2_CMD_REQ, 1);
        for (const char c : command.rsvd) assert(c == 0);
        for (int i = 0; i < LVX2_N_DATA; ++i) command.addData(uint8_t(0));
        bool bounded = false; try { command.addData(uint8_t(0)); } catch (const std::length_error &) { bounded = true; }
        assert(bounded);
    }

    LIVOX2_DATA points(_Livox2 &driver, uint8_t type, uint64_t stamp)
    {
        auto bytes = packet(type, 3, stamp, false);
        if (type == 1)
        {
            LivoxLidarCartesianHighRawPoint raw[3] = {{1000, 0, 0, 255, 0}, {0, 0, 0, 60, 0}, {0, 2000, 0, 128, 0}};
            memcpy(bytes.data() + 36, raw, sizeof(raw));
        }
        else if (type == 2)
        {
            LivoxLidarCartesianLowRawPoint raw[3] = {{100, 0, 0, 255, 0}, {0, 0, 0, 60, 0}, {0, 200, 0, 128, 0}};
            memcpy(bytes.data() + 36, raw, sizeof(raw));
        }
        else
        {
            LivoxLidarSpherPoint raw[3] = {{1000, 9000, 0, 255, 0}, {0, 0, 0, 60, 0}, {2000, 9000, 9000, 128, 0}};
            memcpy(bytes.data() + 36, raw, sizeof(raw));
        }
        LIVOX2_DATA result;
        assert(Livox2TestAccess::data(driver, bytes, result)); return result;
    }

    void testCloud()
    {
        for (uint8_t type = 1; type <= 3; ++type)
        {
            _Livox2 driver; PCLframe cloud; IMUstream imu; BytePacketStream commands;
            Livox2TestAccess::outputs(driver, cloud, imu, commands);
            const uint64_t start = 10000000000ULL;
            Livox2TestAccess::points(driver, points(driver, type, start));
            Livox2TestAccess::points(driver, points(driver, type, start)); // duplicate rejected
            Livox2TestAccess::points(driver, points(driver, type, start - 100)); // reordering rejected
            Livox2TestAccess::points(driver, points(driver, type, start + 100000000));
            vector<GEOMETRY_POINT> actual;
            assert(cloud.get(actual) == start); assert(actual.size() == 2);
            assert((actual[0].m_vP - Vector3f(1, 0, 0)).norm() < 1e-5f);
            assert((actual[1].m_vP - Vector3f(0, 2, 0)).norm() < 1e-5f);
            assert(actual[0].m_tStamp == start); assert(actual[1].m_tStamp == start + 10000);
            assert(actual[0].m_vC.x() == 1.0f);
            json errors;
            assert(Livox2TestAccess::config(driver, {{"vRangeD", {100, 101}}}, errors));
            Livox2TestAccess::points(driver, points(driver, type, start + 200000000));
            Livox2TestAccess::points(driver, points(driver, type, start + 300000000));
            cloud.get(actual); assert(actual.empty()); // filtering every return clears the viewer
            driver.clear(); cloud.get(actual); assert(actual.empty());
        }
    }

    LIVOX2_DATA imuPacket(_Livox2 &driver, uint64_t stamp)
    {
        auto bytes = packet(0, 1, stamp);
        LivoxLidarImuRawPoint raw{0.1f, -0.2f, 0.3f, 0, 0, 1};
        memcpy(bytes.data() + 36, &raw, sizeof(raw));
        put(bytes, 24, CRC::Calculate(bytes.data() + 28, bytes.size() - 28, CRC::CRC_32()), 4);
        LIVOX2_DATA data; assert(Livox2TestAccess::data(driver, bytes, data)); return data;
    }

    void testIMU()
    {
        _Livox2 driver; PCLframe cloud; IMUstream imu; BytePacketStream commands;
        Livox2TestAccess::outputs(driver, cloud, imu, commands);
        const uint64_t stamp = 10000000000ULL;
        auto data = imuPacket(driver, stamp);
        Livox2TestAccess::imu(driver, data);
        Livox2TestAccess::imu(driver, data); // duplicate rejected
        Livox2TestAccess::imu(driver, points(driver, 1, stamp + 1)); // wrong stream type
        deque<IMUstream::IMU_DATA> gyro, acc;
        imu.get(gyro, acc); assert(gyro.size() == 1 && acc.size() == 1);
        assert(gyro[0].m_t == stamp && acc[0].m_t == stamp);
        assert(std::abs(acc[0].m_v.z() - 9.80665f) < 1e-5f);
        assert(std::abs(gyro[0].m_v.y() + 0.2f) < 1e-5f);
        for (int i = 0; i < 10; ++i) Livox2TestAccess::snapshot(driver);
        imu.get(gyro, acc); assert(gyro.size() == 1); // preview does not consume SLAM input
        Livox2TestAccess::imu(driver, imuPacket(driver, stamp + 5000000));
        assert(Livox2TestAccess::snapshot(driver).at("orientationValid") == false); // disconnected preview freshness
        Livox2TestAccess::imu(driver, imuPacket(driver, 1000000000)); // sensor clock restart clears old history
        imu.get(gyro, acc); assert(gyro.size() == 1 && gyro[0].m_t == 1000000000);
        json errors;
        assert(Livox2TestAccess::config(driver, {{"lvxIMUdataEn", 0}}, errors));
        Livox2TestAccess::imu(driver, imuPacket(driver, 1005000000));
        imu.get(gyro, acc); assert(gyro.empty() && acc.empty());
    }

    void testConfigurationAndACK()
    {
        _Livox2 driver; PCLframe cloud; IMUstream imu; BytePacketStream commands;
        Livox2TestAccess::outputs(driver, cloud, imu, commands);
        const json before = Livox2TestAccess::values(driver); json errors;
        assert(!Livox2TestAccess::config(driver, {{"lvxPCLdataType", 99}, {"frameDurationMs", 25}}, errors));
        assert(Livox2TestAccess::values(driver) == before); // validation is transactional
        assert(!Livox2TestAccess::config(driver, {{"lvxHostIP", "1.2.3.4"}}, errors));
        assert(!Livox2TestAccess::config(driver, {{"vRangeD", {10, 1}}}, errors));
        assert(!Livox2TestAccess::config(driver, {{"vColorDefault", {0, "bad", 1}}}, errors));
        assert(!Livox2TestAccess::config(driver, {{"lvxPatternMode", 1}}, errors));
        assert(Livox2TestAccess::config(driver, {{"lvxPCLdataType", 2}, {"frameDurationMs", 25}}, errors));
        Livox2TestAccess::connected(driver);
        Livox2TestAccess::queue(driver, kKeyDetectMode, 1); const uint32_t old = Livox2TestAccess::sequence(driver);
        Livox2TestAccess::queue(driver, kKeyDetectMode, 0); assert(Livox2TestAccess::pending(driver) == 1);
        const uint32_t latest = Livox2TestAccess::sequence(driver); assert(latest != old);
        LIVOX2_CMD ack; ack.init(LVX2_CMD_SET, LVX2_CMD_ACK, old); ack.addData(uint8_t(0)); ack.addData(uint16_t(0));
        Livox2TestAccess::ack(driver, ack); assert(Livox2TestAccess::pending(driver) == 1); // stale ACK ignored
        ack.seq_num = latest; ack.data[0] = 0x20;
        Livox2TestAccess::ack(driver, ack); assert(Livox2TestAccess::failed(driver)); assert(Livox2TestAccess::pending(driver) == 0);
        assert(Livox2TestAccess::config(driver, {{"lvxDetectMode", 0}}, errors)); // retry identical desired value
        assert(Livox2TestAccess::pending(driver) == 6); // full config must be acknowledged
        for (const auto &pending : Livox2TestAccess::pendingCommands(driver))
        {
            ack.seq_num = pending.seq_num;
            ack.data[0] = livox2::u16(pending.data + 4) == kKeyDetectMode ? 0x20 : 0;
            Livox2TestAccess::ack(driver, ack);
        }
        assert(Livox2TestAccess::failed(driver)); assert(Livox2TestAccess::pending(driver) == 0);
        assert(Livox2TestAccess::config(driver, {{"lvxIMUdataEn", 0}}, errors)); // unrelated edit retries failed detect too
        const auto recovery = Livox2TestAccess::pendingCommands(driver);
        assert(recovery.size() == 6);
        bool retriesFailedKey = false;
        for (const auto &pending : recovery)
        {
            retriesFailedKey |= livox2::u16(pending.data + 4) == kKeyDetectMode;
            ack.seq_num = pending.seq_num; ack.data[0] = 0;
            Livox2TestAccess::ack(driver, ack);
        }
        assert(retriesFailedKey && !Livox2TestAccess::failed(driver) && Livox2TestAccess::pending(driver) == 0);
        Livox2TestAccess::info(driver, {0, 0, 1, 0, 3, 1, 0, 255, 255}, 2);
        assert(Livox2TestAccess::observed(driver).empty()); // malformed TLV cannot partially update status
        Livox2TestAccess::info(driver, {0, 0, 1, 0, 3}, 1);
        assert(Livox2TestAccess::observed(driver).at("lvxPCLdataType") == 3);
    }
}
int main()
{
    testProtocol(); testCloud(); testIMU(); testConfigurationAndACK();
    std::cout << "Livox2 packet validation, scan timing, coordinate formats, IMU units, epoch reset and control tests passed\n";
}
#endif
