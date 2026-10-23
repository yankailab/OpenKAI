#include "../../src/Sensor/LiDAR/RoboSense/Airy/AiryProtocol.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace
{
    using namespace kai::airy;
    void put(std::vector<uint8_t> &bytes, size_t offset, uint64_t value, size_t size)
    { for (size_t i = 0; i < size; ++i) bytes[offset + size - 1 - i] = uint8_t(value >> (8 * i)); }
    void floatAt(std::vector<uint8_t> &bytes, size_t offset, float value)
    { uint32_t bits; std::memcpy(&bits, &value, 4); put(bytes, offset, bits, 4); }

    std::vector<uint8_t> difop()
    {
        std::vector<uint8_t> bytes(1248, 0);
        const uint8_t magic[] = {0xa5,0xff,0,0x5a,0x11,0x11,0x55,0x55};
        std::memcpy(bytes.data(), magic, 8);
        put(bytes, 8, 600, 2); put(bytes, 10, 0xc0a801c8, 4); put(bytes, 14, 0xc0a80166, 4);
        const uint8_t mac[] = {0x40,0x2c,0x76,0x08,0x4a,0xcc};
        std::memcpy(bytes.data() + 18, mac, 6);
        put(bytes, 24, 6699, 2); put(bytes, 28, 7788, 2); put(bytes, 34, 36000, 2);
        bytes[40] = 1; bytes[44] = 2; bytes[45] = 3; bytes[50] = 4; bytes[55] = 5;
        bytes[289] = 0; put(bytes, 292, 0x12345678abcdULL, 6);
        bytes[300] = 3; bytes[301] = 3; bytes[302] = 1;
        put(bytes, 303, 4328719365ULL, 6); put(bytes, 309, 123456, 4); put(bytes, 373, 600, 2);
        for (size_t i = 0; i < 96; ++i) put(bytes, 469 + i * 3, i * 10, 2);
        bytes[759] = 1; put(bytes, 760, 174, 2); // Manual horizontal example: -1.74 degrees.
        put(bytes, 1044, 1200, 2); put(bytes, 1066, 2400, 2); put(bytes, 1068, 1210, 2); put(bytes, 1080, 2567, 2);
        floatAt(bytes, 1104, 1); floatAt(bytes, 1108, .01f); floatAt(bytes, 1112, -.02f); floatAt(bytes, 1116, .03f);
        bytes[1246] = 0x0f; bytes[1247] = 0xf0;
        return bytes;
    }

    std::vector<uint8_t> msop(uint8_t model = 2, uint8_t returns = 0)
    {
        std::vector<uint8_t> bytes(1248, 0);
        bytes[0] = 0x55; bytes[1] = 0xaa; bytes[2] = 0x05; bytes[3] = 0x5a;
        put(bytes, 12, 0x1234, 4); bytes[17] = returns;
        put(bytes, 20, 2, 6); put(bytes, 26, 100, 4);
        bytes[31] = 0x31; bytes[32] = model;
        bytes[38] = 0; bytes[39] = 10; // 20 Celsius, signed-magnitude little-endian bit field.
        bytes[40] = 0x80; bytes[41] = 0x81; // -3 Celsius.
        const unsigned group = (model == 2 ? 2 : 1) * (returns == 3 ? 2 : 1);
        for (unsigned block = 0; block < 8; ++block)
        {
            const unsigned offset = 42 + 148 * block;
            bytes[offset] = 0xff; bytes[offset + 1] = 0xee;
            put(bytes, offset + 2, 9000 + 40 * (block / group), 2);
            for (unsigned lane = 0; lane < 48; ++lane)
            {
                put(bytes, offset + 4 + lane * 3, 0xc000 | 200, 2);
                bytes[offset + 6 + lane * 3] = 123;
            }
        }
        bytes[1230] = 0; bytes[1231] = 0xff;
        return bytes;
    }

    std::vector<uint8_t> imu()
    {
        std::vector<uint8_t> bytes(51, 0);
        bytes[0] = 0xaa; bytes[1] = 0x55; bytes[2] = 0x5a; bytes[3] = 0x05;
        put(bytes, 4, 336, 6); put(bytes, 10, 489352, 4);
        put(bytes, 14, uint32_t(-22), 4); put(bytes, 18, 106, 4); put(bytes, 22, uint32_t(-8192), 4);
        put(bytes, 26, uint32_t(-16384), 4); put(bytes, 30, 8192, 4); put(bytes, 34, 0, 4);
        put(bytes, 38, 3737, 4); bytes[42] = 3; bytes[43] = 1; bytes[44] = 1;
        bytes[45] = 0xed; bytes[46] = 4; bytes[47] = 1; bytes[48] = 0;
        bytes[49] = 0xf0; bytes[50] = 0x0f;
        return bytes;
    }

    void testDifop()
    {
        auto bytes = difop(); Difop d; std::string error = "old error";
        assert(decodeDifop(bytes.data(), bytes.size(), d, &error)); assert(error.empty());
        assert(d.sensorIp == "192.168.1.200" && d.destinationIp == "192.168.1.102");
        assert(d.mac == "40:2C:76:08:4A:CC" && d.serial == "12345678ABCD");
        assert(d.msopPort == 6699 && d.difopPort == 7788 && d.rpm == 600 && d.motorSpeed == 600);
        assert(d.mainFirmware == "01:00:00:00:02" && d.baseFirmware == "03:00:00:00:00");
        assert(d.appFirmware == "04:00:00:00:00" && d.motorFirmware == "05:00:00:00:00");
        assert(d.timestampNs == 4328719365123456000ULL && d.timeMode == 3 && d.timeStatus == 1);
        assert(d.returnMode == 3 && d.calibration.valid && d.calibration.installMode == 0);
        assert(d.calibration.vertical[95] == 950 && d.calibration.horizontal[1] == -174);
        assert(d.calibration.rings[95] == 95 && d.calibration.rings[0] == 0);
        assert(d.mainVoltage == 12 && d.inputVoltage == 24 && std::abs(d.baseVoltage - 12.1) < 1e-9);
        assert(std::abs(d.temperature - 25.67) < 1e-9);
        assert(d.imuCalibrationValid && d.imuQuaternion[3] == 1);
        assert(std::abs(d.imuTranslation[1] + .02) < 1e-8);
        assert(std::memcmp(d.raw.data(), bytes.data(), bytes.size()) == 0);
        const auto saved = d.calibration;
        for (size_t size = 0; size < bytes.size(); ++size) assert(!decodeDifop(bytes.data(), size, d));
        assert(!decodeDifop(nullptr, 1248, d) && !decodeDifop(bytes.data(), 1249, d));
        for (const unsigned offset : {0u,7u,1246u,1247u})
        { auto bad = bytes; bad[offset] ^= 1; assert(!decodeDifop(bad.data(), bad.size(), d)); }
        auto bad = bytes; bad[468] = 2; assert(!decodeDifop(bad.data(), bad.size(), d));
        assert(d.calibration == saved); // Invalid calibration never replaces the last good one.
        bad = bytes; put(bad, 469, 9001, 2); assert(!decodeDifop(bad.data(), bad.size(), d));
        bad = bytes; put(bad, 309, 1000000, 4); assert(!decodeDifop(bad.data(), bad.size(), d));
        bad = bytes; put(bad, 303, 0xffffffffffffULL, 6); assert(!decodeDifop(bad.data(), bad.size(), d));
        bad = bytes; put(bad, 34, 36001, 2); assert(!decodeDifop(bad.data(), bad.size(), d));
        bad = bytes; std::fill(bad.begin() + 468, bad.begin() + 1044, 0);
        assert(decodeDifop(bad.data(), bad.size(), d) && !d.calibration.valid);
        bad = bytes; floatAt(bad, 1092, std::numeric_limits<float>::quiet_NaN());
        assert(decodeDifop(bad.data(), bad.size(), d) && !d.imuCalibrationValid);
        bad = bytes; bad[289] = 1;
        assert(decodeDifop(bad.data(), bad.size(), d) && d.calibration != saved);
    }

    void testMsop()
    {
        auto calibrationBytes = difop(); Difop d;
        assert(decodeDifop(calibrationBytes.data(), calibrationBytes.size(), d));
        auto bytes = msop(); Msop m; std::string error;
        assert(decodeMsop(bytes.data(), bytes.size(), d.calibration, m, &error));
        assert(m.sequence == 0x1234 && m.sequenceValid && m.lidarType == 0x31 && m.model == 2 && !m.dual);
        assert(m.timestampNs == 2000100000ULL && m.temperature == 20 && m.topTemperature == -3);
        const double radial = std::sqrt(.0075 * .0075 + .00664 * .00664);
        assert(std::abs(m.points[0].x) < 1e-6 && std::abs(m.points[0].y + 1 + radial) < 1e-6);
        assert(std::abs(m.points[0].z - .04532) < 1e-6 && m.points[0].range == 1);
        assert(m.points[0].feature == 3 && m.points[0].intensity == 123);
        assert(m.points[48].channel == 48 && m.points[48].ring == 48 && m.points[48].offsetNs == 41888);
        assert(m.points[8].offsetNs == 5712 && m.points[96].offsetNs == 111080);
        assert(m.points[383].offsetNs == 426536 && m.points[383].timestampNs == 2000526536ULL);
        assert(m.blockOffsetsNs[0] == 0 && m.blockOffsetsNs[1] == 0 && m.blockOffsetsNs[7] == 333240);
        // Encoder-clockwise yaw, horizontal calibration, lens and vertical angles are applied independently.
        const double azimuth = (9000 - 174) * kPi / 18000;
        const double elevation = 10 * kPi / 18000;
        assert(std::abs(m.points[1].x - std::cos(elevation) * std::cos(azimuth)) < 1e-6);
        assert(std::abs(m.points[1].z - (std::sin(elevation) + .04532)) < 1e-6);
        auto unaligned = bytes; unaligned.insert(unaligned.begin(), 0);
        assert(decodeMsop(unaligned.data() + 1, bytes.size(), d.calibration, m));
        for (size_t size = 0; size < bytes.size(); ++size) assert(!decodeMsop(bytes.data(), size, d.calibration, m));
        assert(!decodeMsop(nullptr, 1248, d.calibration, m) && !decodeMsop(bytes.data(), 1249, d.calibration, m));
        for (const unsigned offset : {0u,3u,42u,43u,1078u,1230u,1231u})
        { auto bad = bytes; bad[offset] ^= 1; assert(!decodeMsop(bad.data(), bad.size(), d.calibration, m)); }
        auto bad = bytes; bad[31] = 0x62; assert(!decodeMsop(bad.data(), bad.size(), d.calibration, m));
        bad = bytes; bad[32] = 3; assert(!decodeMsop(bad.data(), bad.size(), d.calibration, m));
        bad = bytes; bad[16] = 1; assert(!decodeMsop(bad.data(), bad.size(), d.calibration, m));
        bad = bytes; put(bad, 44, 36000, 2); assert(!decodeMsop(bad.data(), bad.size(), d.calibration, m));
        bad = bytes; put(bad, 26, 1000000, 4); assert(!decodeMsop(bad.data(), bad.size(), d.calibration, m));
        bad = bytes; put(bad, 20, UINT64_MAX / 1000000000, 6); put(bad, 26, (UINT64_MAX % 1000000000) / 1000, 4);
        assert(!decodeMsop(bad.data(), bad.size(), d.calibration, m));
        Calibration empty; assert(!decodeMsop(bytes.data(), bytes.size(), empty, m));
        bad = bytes; bad[31] = 0x10; assert(decodeMsop(bad.data(), bad.size(), d.calibration, m));
        put(bad, 12, 65535, 4); assert(decodeMsop(bad.data(), bad.size(), d.calibration, m) && m.sequenceValid);
        put(bad, 12, 65536, 4); assert(decodeMsop(bad.data(), bad.size(), d.calibration, m) && !m.sequenceValid);
    }

    void testModelsAndReturns()
    {
        Difop d; const auto db = difop(); assert(decodeDifop(db.data(), db.size(), d));
        Msop m; auto bytes = msop(2, 3);
        assert(decodeMsop(bytes.data(), bytes.size(), d.calibration, m) && m.dual);
        assert(m.points[0].timestampNs == m.points[96].timestampNs); // Both returns describe the same firing.
        assert(m.points[48].timestampNs == m.points[144].timestampNs);
        assert(m.points[192].offsetNs == 111080 && m.points[383].offsetNs == 204376);
        bytes = msop(1);
        assert(decodeMsop(bytes.data(), bytes.size(), d.calibration, m));
        assert(m.points[4].offsetNs == 7616 && m.points[48].channel == 0 && m.points[48].offsetNs == 111080);
        assert(m.points[383].offsetNs == 874664);
        bytes = msop(1, 3); assert(decodeMsop(bytes.data(), bytes.size(), d.calibration, m));
        assert(m.points[0].timestampNs == m.points[48].timestampNs && m.points[96].offsetNs == 111080);
        bytes = msop(2); d.calibration.installMode = 1;
        assert(decodeMsop(bytes.data(), bytes.size(), d.calibration, m));
        assert(m.points[8].offsetNs == 11424 && m.points[383].offsetNs == 432248);
        d.calibration.installMode = 2;
        assert(decodeMsop(bytes.data(), bytes.size(), d.calibration, m));
        assert(m.points[8].offsetNs == 7616 && m.points[383].offsetNs == 430344);
        d.calibration.installMode = 4;
        assert(!decodeMsop(bytes.data(), bytes.size(), d.calibration, m));
        d.calibration.installMode = 0;
        for (unsigned block = 0; block < 8; ++block) put(bytes, 44 + block * 148, (35980 + 40 * (block / 2)) % 36000, 2);
        assert(decodeMsop(bytes.data(), bytes.size(), d.calibration, m));
        assert(m.azimuths[0] == 35980 && m.azimuths[2] == 20 && m.blockOffsetsNs[2] == 111080);
        // A configured 90-degree blind interval delays the next firing by 25ms at 600rpm.
        d.calibration.fovStart = 9000; d.calibration.fovEnd = 36000;
        for (unsigned block = 0; block < 8; ++block) put(bytes, 44 + block * 148, block < 2 ? 35980 : 9000 + (block / 2 - 1) * 40, 2);
        assert(decodeMsop(bytes.data(), bytes.size(), d.calibration, m));
        assert(m.blockOffsetsNs[2] == 25000000);
    }

    void testImu()
    {
        auto bytes = imu(); Imu decoded;
        assert(decodeImu(bytes.data(), bytes.size(), decoded));
        assert(decoded.sequence == 66797 && decoded.timestampNs == 336489352000ULL);
        assert(decoded.outputRate == 3 && decoded.accelRange == 1 && decoded.gyroRange == 1);
        assert(decoded.temperatureRaw == 3737);
        assert(std::abs(decoded.acceleration[2] + 9.80665) < 1e-10);
        assert(std::abs(decoded.angularVelocity[0] + 250 * kPi / 180) < 1e-10);
        assert(std::abs(decoded.angularVelocity[1] - 125 * kPi / 180) < 1e-10);
        for (size_t size = 0; size < bytes.size(); ++size) assert(!decodeImu(bytes.data(), size, decoded));
        assert(!decodeImu(nullptr, 51, decoded) && !decodeImu(bytes.data(), 52, decoded));
        for (const unsigned offset : {0u,3u,49u,50u})
        { auto bad = bytes; bad[offset] ^= 1; assert(!decodeImu(bad.data(), bad.size(), decoded)); }
        for (const unsigned offset : {42u,43u,44u})
        { auto bad = bytes; bad[offset] = 4; assert(!decodeImu(bad.data(), bad.size(), decoded)); }
        auto bad = bytes; put(bad, 10, 1000000, 4); assert(!decodeImu(bad.data(), bad.size(), decoded));
        bad = bytes; put(bad, 4, 0xffffffffffffULL, 6); assert(!decodeImu(bad.data(), bad.size(), decoded));
        for (unsigned scale = 0; scale < 4; ++scale)
        {
            bytes[43] = bytes[44] = static_cast<uint8_t>(scale);
            assert(decodeImu(bytes.data(), bytes.size(), decoded));
            assert(std::abs(decoded.acceleration[2] + (2u << scale) * 9.80665 / 4) < 1e-10);
        }
    }

    void testCapture(const std::string &directory)
    {
        std::ifstream calibrationFile(directory + "/7788.bin", std::ios::binary);
        std::vector<uint8_t> raw((std::istreambuf_iterator<char>(calibrationFile)), {});
        Difop d; assert(decodeDifop(raw.data(), raw.size(), d) && d.calibration.valid);
        std::ifstream packets(directory + "/packets.bin", std::ios::binary);
        assert(packets.good()); uint8_t header[12]; size_t msopCount = 0, imuCount = 0, difopCount = 0;
        while (packets.read(reinterpret_cast<char *>(header), 12))
        {
            const unsigned port = header[8] | (unsigned(header[9]) << 8);
            const unsigned size = header[10] | (unsigned(header[11]) << 8);
            std::vector<uint8_t> packet(size);
            assert(packets.read(reinterpret_cast<char *>(packet.data()), size));
            if (port == 6699) { Msop m; assert(decodeMsop(packet.data(), size, d.calibration, m)); ++msopCount; }
            else if (port == 7788) { assert(decodeDifop(packet.data(), size, d)); ++difopCount; }
            else if (port == 6688) { Imu i; assert(decodeImu(packet.data(), size, i)); ++imuCount; }
        }
        assert(packets.gcount() == 0 && msopCount && difopCount && imuCount);
        std::cout << "Captured packets decoded: " << msopCount << " MSOP, " << difopCount << " DIFOP, " << imuCount << " IMU\n";
    }
}

int main(int argc, char **argv)
{
    testDifop(); testMsop(); testModelsAndReturns(); testImu();
    if (argc > 1) testCapture(argv[1]);
    std::cout << "Airy protocol tests passed\n";
}
