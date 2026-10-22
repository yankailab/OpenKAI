#ifndef OPENKAI_E1R_PROTOCOL_H
#define OPENKAI_E1R_PROTOCOL_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

namespace kai
{
namespace e1r
{
    // E1R User Guide v1.0 (2025-01-02), sections 4.3.1 and 4.3.2.
    // These are UDP payload sizes, excluding Ethernet/IP/UDP headers.
    constexpr std::size_t kMsopSize = 1200;
    constexpr std::size_t kDifopSize = 256;
    constexpr std::size_t kPointCount = 96;

    // Read bytes explicitly: network fields may be unaligned and are big-endian.
    inline uint16_t u16(const uint8_t *p)
    { return (uint16_t(p[0]) << 8) | uint16_t(p[1]); }

    inline uint32_t u32(const uint8_t *p)
    { return (uint32_t(u16(p)) << 16) | uint32_t(u16(p + 2)); }

    inline int32_t i16(const uint8_t *p)
    {
        const uint16_t value = u16(p);
        return value < 0x8000 ? int32_t(value) : int32_t(value) - 65536;
    }

    inline bool fail(std::string *error, const char *message)
    {
        if (error) *error = message;
        return false;
    }

    // Six bytes of seconds followed by four bytes of microseconds, both BE.
    // Keep the timestamp integral to preserve microsecond precision.
    inline bool timestamp(const uint8_t *p, uint64_t &result)
    {
        uint64_t seconds = 0;
        for (std::size_t i = 0; i < 6; ++i) seconds = (seconds << 8) | p[i];
        const uint32_t micros = u32(p + 6);
        if (micros >= 1000000 ||
            seconds > (std::numeric_limits<uint64_t>::max() - micros) / 1000000)
            return false;
        result = seconds * 1000000 + micros;
        return true;
    }

    struct Point
    {
        float x = 0, y = 0, z = 0, range = 0;
        uint8_t intensity = 0, attribute = 0;
        uint16_t offsetUs = 0;
        uint64_t timestampUs = 0;
    };

    struct Msop
    {
        // PktCnt starts at zero for each frame; its maximum is not specified.
        uint16_t sequence = 0, version = 0;
        uint8_t returnMode = 0, timeMode = 0;
        bool frameSync = false;
        int temperature = 0;
        uint64_t timestampUs = 0;
        std::array<Point, kPointCount> points{};
        uint16_t dataLength = 0, counter = 0;
        uint32_t dataId = 0, crc32 = 0;
    };

    inline bool decodeMsop(const uint8_t *p, std::size_t size, Msop &result,
                           std::string *error = nullptr)
    {
        static const uint8_t magic[] = {0x55, 0xaa, 0x5a, 0xa5};
        if (!p || size != kMsopSize) return fail(error, "E1R MSOP must contain 1200 bytes");
        if (std::memcmp(p, magic, sizeof(magic))) return fail(error, "Invalid E1R MSOP header");
        if (p[30] != 0x62) return fail(error, "MSOP is not an E1R packet (LiDAR type must be 0x62)");

        Msop decoded;
        decoded.sequence = u16(p + 4);
        decoded.version = u16(p + 6);
        decoded.returnMode = p[8];
        decoded.timeMode = p[9];
        if (!timestamp(p + 10, decoded.timestampUs)) return fail(error, "Invalid E1R MSOP timestamp");
        decoded.frameSync = p[20] == 1;
        decoded.temperature = int(p[31]) - 80;

        for (std::size_t i = 0; i < kPointCount; ++i)
        {
            const uint8_t *block = p + 32 + 12 * i;
            Point &point = decoded.points[i];
            point.offsetUs = u16(block);
            if (decoded.timestampUs > std::numeric_limits<uint64_t>::max() - point.offsetUs)
                return fail(error, "E1R point timestamp overflow");
            point.timestampUs = decoded.timestampUs + point.offsetUs;
            point.range = float(u16(block + 2)) * 0.005f;
            const float scale = point.range / 32768.0f;
            point.x = float(i16(block + 4)) * scale;
            point.y = float(i16(block + 6)) * scale;
            point.z = float(i16(block + 8)) * scale;
            point.intensity = block[10];
            point.attribute = block[11];
        }

        // The manual lists E2E Profile4 trailer fields but does not specify CRC
        // coverage/initialization. Preserve them for diagnostics; do not claim
        // CRC integrity or reject firmware that leaves the trailer unpopulated.
        decoded.dataLength = u16(p + 1188);
        decoded.counter = u16(p + 1190);
        decoded.dataId = u32(p + 1192);
        decoded.crc32 = u32(p + 1196);
        result = decoded;
        if (error) error->clear();
        return true;
    }

    inline std::string ipString(const uint8_t *p)
    {
        return std::to_string(p[0]) + "." + std::to_string(p[1]) + "." +
               std::to_string(p[2]) + "." + std::to_string(p[3]);
    }

    inline std::string hexString(const uint8_t *p, std::size_t size, bool separators)
    {
        static const char digits[] = "0123456789ABCDEF";
        std::string result;
        result.reserve(size * (separators ? 3 : 2));
        for (std::size_t i = 0; i < size; ++i)
        {
            if (separators && i) result += ':';
            result += digits[p[i] >> 4];
            result += digits[p[i] & 15];
        }
        return result;
    }

    struct Difop
    {
        std::string sensorIp, netmask, mac, msopIp, difopIp, serial, firmware;
        uint16_t msopLocalPort = 0, msopPort = 0, difopLocalPort = 0, difopPort = 0;
        uint8_t frameRate = 0, returnMode = 0, timeMode = 0, timeStatus = 0, phyMode = 0;
        uint64_t timestampUs = 0;
        std::array<uint8_t, kDifopSize> raw{};
    };

    inline bool decodeDifop(const uint8_t *p, std::size_t size, Difop &result,
                            std::string *error = nullptr)
    {
        // The guide names the recognition header without listing its bytes.
        // Verified against the vendor's RSE1 decoder (reference only):
        // github.com/RoboSense-LiDAR/rs_driver/.../decoder/decoder_RSE1.hpp
        static const uint8_t magic[] = {0xa5, 0xff, 0x00, 0x5a, 0x11, 0x11, 0x55, 0x55};
        if (!p || size != kDifopSize) return fail(error, "E1R DIFOP must contain 256 bytes");
        if (std::memcmp(p, magic, sizeof(magic))) return fail(error, "Invalid E1R DIFOP header");

        Difop decoded;
        decoded.firmware = std::to_string(p[16]) + "." + std::to_string(p[17]) + "." + std::to_string(p[18]);
        decoded.serial = hexString(p + 20, 6, false);
        decoded.sensorIp = ipString(p + 44);
        decoded.netmask = ipString(p + 48);
        decoded.mac = hexString(p + 52, 6, true);
        decoded.msopIp = ipString(p + 58);
        decoded.msopLocalPort = u16(p + 62);
        decoded.msopPort = u16(p + 64);
        decoded.difopIp = ipString(p + 66);
        decoded.difopLocalPort = u16(p + 70);
        decoded.difopPort = u16(p + 72);
        decoded.frameRate = p[99];
        decoded.returnMode = p[100];
        decoded.timeMode = p[101];
        decoded.timeStatus = p[102];
        if (!timestamp(p + 103, decoded.timestampUs)) return fail(error, "Invalid E1R DIFOP timestamp");
        decoded.phyMode = p[113];
        std::memcpy(decoded.raw.data(), p, size);
        result = decoded;
        if (error) error->clear();
        return true;
    }

    // E1R's published protocol defines MSOP and DIFOP as output only. Do not
    // reuse another RoboSense model's UCWP messages: no E1R configuration-write
    // format, port, checksum or acknowledgement is documented in this guide.
}
}
#endif
