// Standalone: g++ -std=c++11 -Wall -Wextra -Werror -pedantic
//   test/Sensor/E1RProtocol.cpp -o /tmp/openkai-e1r-protocol-test
#include "../../src/Sensor/LiDAR/RoboSense/E1R/E1RProtocol.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

namespace
{
    using namespace kai::e1r;

    void put(std::vector<uint8_t> &bytes, std::size_t offset, uint64_t value, std::size_t length)
    {
        for (std::size_t i = 0; i < length; ++i)
            bytes[offset + length - 1 - i] = uint8_t(value >> (i * 8));
    }

    std::vector<uint8_t> msop()
    {
        std::vector<uint8_t> bytes(1200, 0);
        bytes[0] = 0x55; bytes[1] = 0xaa; bytes[2] = 0x5a; bytes[3] = 0xa5;
        bytes[4] = 0x12; bytes[5] = 0x34; bytes[6] = 0x01; bytes[7] = 0x02;
        bytes[8] = 4; bytes[9] = 3; bytes[20] = 1; bytes[30] = 0x62; bytes[31] = 105;
        // Use seconds above 32 bits, preventing a truncation bug from passing.
        put(bytes, 10, 0x0102030405ULL, 6); put(bytes, 16, 999990, 4);
        // Manual example distance 0x03fc = 5.10 metres, signed Q15 directions.
        bytes[32] = 0x01; bytes[33] = 0x02;
        bytes[34] = 0x03; bytes[35] = 0xfc;
        bytes[36] = 0x40; bytes[37] = 0x00;
        bytes[38] = 0x80; bytes[39] = 0x00;
        bytes[40] = 0x7f; bytes[41] = 0xff;
        bytes[42] = 255; bytes[43] = 2;
        // Last block catches off-by-one block counts and negative sign handling.
        put(bytes, 1172, 65535, 2); put(bytes, 1174, 200, 2);
        put(bytes, 1176, 0xffff, 2); put(bytes, 1178, 0x8001, 2);
        bytes[1182] = 42; bytes[1183] = 1;
        put(bytes, 1188, 1200, 2); put(bytes, 1190, 0xabcd, 2);
        put(bytes, 1192, 0x00000e5c, 4); put(bytes, 1196, 0xdeadbeef, 4);
        return bytes;
    }

    std::vector<uint8_t> difop()
    {
        std::vector<uint8_t> bytes(256, 0xee); // Reserved fields need not be zero.
        const uint8_t magic[] = {0xa5, 0xff, 0x00, 0x5a, 0x11, 0x11, 0x55, 0x55};
        std::memcpy(bytes.data(), magic, sizeof(magic));
        bytes[16] = 1; bytes[17] = 10; bytes[18] = 3;
        const uint8_t serial[] = {0x00, 0x01, 0xab, 0xcd, 0xef, 0xff};
        std::memcpy(bytes.data() + 20, serial, sizeof(serial));
        const uint8_t network[] = {
            192,168,1,200, 255,255,0,0, 0x02,0x10,0x20,0x30,0x40,0xff,
            192,168,1,102, 0x1a,0x2b, 0x23,0x45,
            239,1,2,3, 0x1e,0x6c, 0x34,0x56};
        std::memcpy(bytes.data() + 44, network, sizeof(network));
        bytes[99] = 10; bytes[100] = 4; bytes[101] = 2; bytes[102] = 1;
        put(bytes, 103, 0x0102030405ULL, 6); put(bytes, 109, 123456, 4);
        bytes[113] = 2;
        return bytes;
    }

    void testMsop()
    {
        const auto bytes = msop();
        Msop decoded;
        std::string error = "previous error";
        assert(decodeMsop(bytes.data(), bytes.size(), decoded, &error));
        assert(error.empty());
        assert(decoded.sequence == 0x1234 && decoded.version == 0x0102);
        assert(decoded.returnMode == 4 && decoded.timeMode == 3 && decoded.frameSync);
        assert(decoded.temperature == 25);
        assert(decoded.timestampUs == 4328719365999990ULL);
        assert(std::fabs(decoded.points[0].range - 5.1f) < 0.00001f);
        assert(std::fabs(decoded.points[0].x - 2.55f) < 0.00001f);
        assert(std::fabs(decoded.points[0].y + 5.1f) < 0.00001f);
        assert(std::fabs(decoded.points[0].z - 5.09984436f) < 0.00001f);
        assert(decoded.points[0].offsetUs == 258);
        assert(decoded.points[0].timestampUs == 4328719366000248ULL);
        assert(decoded.points[0].intensity == 255 && decoded.points[0].attribute == 2);
        assert(decoded.points[1].range == 0 && decoded.points[1].x == 0);
        assert(decoded.points[95].timestampUs == 4328719366065525ULL);
        assert(decoded.points[95].x == -1.0f / 32768);
        assert(decoded.points[95].y == -32767.0f / 32768);
        assert(decoded.points[95].intensity == 42 && decoded.points[95].attribute == 1);
        assert(decoded.dataLength == 1200 && decoded.counter == 0xabcd);
        assert(decoded.dataId == 0xe5c && decoded.crc32 == 0xdeadbeef);

        // Do not assume the incoming buffer has any particular alignment.
        auto unaligned = bytes; unaligned.insert(unaligned.begin(), 0xff);
        assert(decodeMsop(unaligned.data() + 1, bytes.size(), decoded));
        for (std::size_t size = 0; size < bytes.size(); ++size)
            assert(!decodeMsop(bytes.data(), size, decoded));
        assert(!decodeMsop(nullptr, 1200, decoded, &error) && !error.empty());
        assert(!decodeMsop(bytes.data(), 1201, decoded));
        for (std::size_t i = 0; i < 4; ++i)
        {
            auto bad = bytes; bad[i] ^= 1;
            assert(!decodeMsop(bad.data(), bad.size(), decoded));
        }
        auto bad = bytes; bad[30] = 0x61;
        assert(!decodeMsop(bad.data(), bad.size(), decoded));
        bad = bytes; put(bad, 16, 1000000, 4);
        decoded.sequence = 9;
        assert(!decodeMsop(bad.data(), bad.size(), decoded));
        assert(decoded.sequence == 9); // Failed decoding is transactional.
        bad = bytes; put(bad, 10, 0xffffffffffffULL, 6);
        assert(!decodeMsop(bad.data(), bad.size(), decoded));
        bad = bytes;
        put(bad, 10, UINT64_MAX / 1000000, 6);
        put(bad, 16, UINT64_MAX % 1000000, 4);
        assert(!decodeMsop(bad.data(), bad.size(), decoded)); // Point offset overflow.
        auto edge = bytes; put(edge, 10, 0, 6); put(edge, 16, 0, 4);
        put(edge, 4, 0, 2); edge[31] = 0;
        assert(decodeMsop(edge.data(), edge.size(), decoded));
        assert(decoded.timestampUs == 0 && decoded.sequence == 0 && decoded.temperature == -80);
        put(edge, 4, 65535, 2); edge[31] = 255;
        assert(decodeMsop(edge.data(), edge.size(), decoded));
        assert(decoded.sequence == 65535 && decoded.temperature == 175);
    }

    void testDifop()
    {
        auto bytes = difop();
        Difop decoded;
        std::string error;
        assert(decodeDifop(bytes.data(), bytes.size(), decoded, &error));
        assert(error.empty());
        assert(decoded.sensorIp == "192.168.1.200" && decoded.netmask == "255.255.0.0");
        assert(decoded.msopIp == "192.168.1.102" && decoded.difopIp == "239.1.2.3");
        assert(decoded.mac == "02:10:20:30:40:FF" && decoded.serial == "0001ABCDEFFF");
        assert(decoded.firmware == "1.10.3");
        assert(decoded.msopLocalPort == 6699 && decoded.msopPort == 0x2345);
        assert(decoded.difopLocalPort == 7788 && decoded.difopPort == 0x3456);
        assert(decoded.frameRate == 10 && decoded.returnMode == 4);
        assert(decoded.timeMode == 2 && decoded.timeStatus == 1 && decoded.phyMode == 2);
        assert(decoded.timestampUs == 4328719365123456ULL);
        assert(std::memcmp(decoded.raw.data(), bytes.data(), bytes.size()) == 0);
        for (std::size_t size = 0; size < bytes.size(); ++size)
            assert(!decodeDifop(bytes.data(), size, decoded));
        assert(!decodeDifop(nullptr, 256, decoded));
        assert(!decodeDifop(bytes.data(), 257, decoded));
        for (std::size_t i = 0; i < 8; ++i)
        {
            auto bad = bytes; bad[i] ^= 1;
            assert(!decodeDifop(bad.data(), bad.size(), decoded));
        }
        auto bad = bytes; put(bad, 109, 1000000, 4);
        assert(!decodeDifop(bad.data(), bad.size(), decoded));
        assert(decoded.sensorIp == "192.168.1.200");
        bad = bytes; put(bad, 103, 0xffffffffffffULL, 6);
        assert(!decodeDifop(bad.data(), bad.size(), decoded));
        // Unknown enum values are telemetry, not a reason to lose the packet.
        bytes[99] = bytes[100] = bytes[101] = bytes[102] = bytes[113] = 255;
        assert(decodeDifop(bytes.data(), bytes.size(), decoded));
        assert(decoded.phyMode == 255 && decoded.timeMode == 255);
    }
}

int main()
{
    testMsop();
    testDifop();
    std::cout << "E1R protocol tests passed\n";
}
