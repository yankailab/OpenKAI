#ifndef OPENKAI_LIVOX1_PROTOCOL_H
#define OPENKAI_LIVOX1_PROTOCOL_H

// Standalone implementation of the public Livox legacy Ethernet protocol (v1.3).
// https://github.com/Livox-SDK/Livox-SDK/wiki/Livox-SDK-Communication-Protocol
// No Livox SDK headers, binaries or packed native structs are required.
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace kai::livox1
{
    constexpr size_t CommandHeaderSize = 11, CommandOverhead = 15, DataHeaderSize = 18;
    constexpr size_t MaxPacketSize = 1400;
    enum Device : uint8_t { Hub = 0, Mid40 = 1, Tele15 = 2, Horizon = 3, Mid70 = 6, Avia = 7, Unknown = 255 };
    enum CommandType : uint8_t { Request = 0, Ack = 1, Message = 2 };
    enum General : uint8_t { Broadcast = 0, Handshake = 1, DeviceInfo = 2, Heartbeat = 3,
        Sampling = 4, Coordinate = 5, Disconnect = 6, Abnormal = 7, WriteParameter = 11, ReadParameter = 12 };
    enum Lidar : uint8_t { WorkMode = 0, SetFan = 4, GetFan = 5, SetReturn = 6, GetReturn = 7, SetImu = 8, GetImu = 9 };
    inline uint16_t u16(const uint8_t *p) { return uint16_t(p[0]) | uint16_t(p[1]) << 8; }
    inline uint32_t u32(const uint8_t *p) { return uint32_t(u16(p)) | uint32_t(u16(p + 2)) << 16; }
    inline uint64_t u64(const uint8_t *p) { return uint64_t(u32(p)) | uint64_t(u32(p + 4)) << 32; }
    inline int32_t i32(const uint8_t *p) { uint32_t u = u32(p); int32_t v; std::memcpy(&v, &u, sizeof(v)); return v; }
    inline int64_t i64(const uint8_t *p) { uint64_t u = u64(p); int64_t v; std::memcpy(&v, &u, sizeof(v)); return v; }
    inline float f32(const uint8_t *p) { uint32_t u = u32(p); float v; std::memcpy(&v, &u, sizeof(v)); return v; }
    inline void put16(uint8_t *p, uint16_t v) { p[0] = uint8_t(v); p[1] = uint8_t(v >> 8); }
    inline void put32(uint8_t *p, uint32_t v) { put16(p, uint16_t(v)); put16(p + 2, uint16_t(v >> 16)); }
    inline uint16_t crc16(const uint8_t *p, size_t n)
    {
        uint16_t crc = 0x4c49;
        while (n--) { crc ^= *p++; for (unsigned b = 0; b < 8; ++b) crc = (crc >> 1) ^ ((crc & 1) ? 0x8408 : 0); }
        return crc;
    }
    inline uint32_t crc32(const uint8_t *p, size_t n)
    {
        uint32_t crc = 0x564f580a ^ 0xffffffffu;
        while (n--) { crc ^= *p++; for (unsigned b = 0; b < 8; ++b) crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320u : 0); }
        return crc ^ 0xffffffffu;
    }
    inline bool validCommand(const uint8_t *p, size_t n, bool checkCRC = true)
    {
        return p && n >= CommandOverhead && n <= MaxPacketSize && p[0] == 0xaa && p[1] == 1 &&
            u16(p + 2) == n && p[4] <= Message && (!checkCRC ||
            (crc16(p, 7) == u16(p + 7) && crc32(p, n - 4) == u32(p + n - 4)));
    }
    inline std::vector<uint8_t> makeCommand(uint8_t set, uint8_t id, uint8_t type, uint16_t sequence,
        const std::vector<uint8_t> &payload = {})
    {
        if (payload.size() > MaxPacketSize - CommandOverhead || type > Message) throw std::length_error("Invalid Livox1 command");
        std::vector<uint8_t> result(CommandOverhead + payload.size());
        auto *p = result.data(); p[0] = 0xaa; p[1] = 1; put16(p + 2, uint16_t(result.size()));
        p[4] = type; put16(p + 5, sequence); put16(p + 7, crc16(p, 7)); p[9] = set; p[10] = id;
        if (!payload.empty()) std::memcpy(p + 11, payload.data(), payload.size());
        put32(p + result.size() - 4, crc32(p, result.size() - 4));
        return result;
    }
    inline size_t pointSize(uint8_t type)
    { const size_t sizes[] = {13, 9, 14, 10, 28, 16, 24, 42, 22}; return type < 9 ? sizes[type] : 0; }
    inline size_t pointCount(uint8_t type)
    { const size_t counts[] = {100, 100, 96, 96, 48, 48, 1, 30, 30}; return type < 9 ? counts[type] : 0; }
    inline size_t returns(uint8_t type) { return type == 4 || type == 5 ? 2 : type == 7 || type == 8 ? 3 : type == 6 ? 0 : 1; }
    inline bool supportedDevice(uint8_t device) { return device == Mid40 || device == Tele15 || device == Horizon || device == Mid70 || device == Avia; }
    inline bool hasImu(uint8_t device) { return device == Tele15 || device == Horizon || device == Avia; }
    inline bool hasFan(uint8_t device) { return device == Mid40 || device == Tele15 || device == Horizon; }
    inline bool supportsData(uint8_t device, uint8_t type)
    {
        if (!supportedDevice(device) || type > 8) return false;
        if (type <= 1) return device == Mid40;
        if (type == 6) return hasImu(device);
        if (type >= 7) return device == Avia;
        return device != Mid40;
    }
    inline bool timestamp(const uint8_t *p, uint8_t type, uint64_t &out)
    {
        if (!p) return false;
        if (type == 0 || type == 1) { out = u64(p); return true; }
        if (type == 4)
        {
            const int64_t offset = i64(p);
            if (offset <= -1000000000LL) return false;
            // One second of headroom represents compensated IMU timestamps < 0.
            // The driver unwraps subsequent PPS seconds using one shared clock.
            // Positive offsets can exceed one second when the PPS signal is
            // lost. Perform the addition unsigned to avoid int64 overflow.
            out = offset >= 0 ? uint64_t(offset) + 1000000000ULL : uint64_t(1000000000LL + offset);
            return true;
        }
        if (type != 3) return false;
        const int year = 2000 + p[0], month = p[1], day = p[2], hour = p[3];
        const bool leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
        const int days[] = {31, 28 + int(leap), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        const uint64_t micros = u32(p + 4);
        if (month < 1 || month > 12 || day < 1 || day > days[month - 1] || hour > 23 || micros >= 3600000000ULL) return false;
        int64_t totalDays = 0;
        for (int y = 1970; y < year; ++y) totalDays += 365 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
        for (int m = 1; m < month; ++m) totalDays += days[m - 1];
        const uint64_t seconds = uint64_t((totalDays + day - 1) * 86400 + hour * 3600);
        if (seconds > (std::numeric_limits<uint64_t>::max() - micros * 1000) / 1000000000ULL) return false;
        out = seconds * 1000000000ULL + micros * 1000; return true;
    }
    inline bool validData(const uint8_t *p, size_t n)
    {
        if (!p || n < DataHeaderSize || n > MaxPacketSize || p[0] != 5 || p[1] < 1 || p[1] > 9 || p[2] < 1 || p[2] > 3 || p[9] > 8) return false;
        uint64_t stamp;
        return n == DataHeaderSize + pointSize(p[9]) * pointCount(p[9]) && timestamp(p + 10, p[8], stamp) &&
            stamp <= std::numeric_limits<uint64_t>::max() - 1000000ULL;
    }
    inline uint64_t pointTime(uint64_t first, uint8_t device, uint8_t type, size_t index)
    {
        const size_t echoes = returns(type);
        const uint64_t rate = device == Mid40 || device == Mid70 ? 100000 : 240000;
        // Echoes of one laser firing have identical times. Integer division is
        // performed once, avoiding cumulative rounding of 4.166666... us.
        return first + (echoes ? (index / echoes) * 1000000000ULL / rate : 0);
    }
    struct Point { float x = 0, y = 0, z = 0; uint8_t reflectivity = 0, tag = 0; };
    inline bool decodePoint(const uint8_t *p, uint8_t type, size_t echo, Point &point)
    {
        if (!p || type > 8 || echo >= returns(type)) return false;
        point = {};
        if (type == 0 || type == 2 || type == 4 || type == 7)
        {
            p += echo * 14;
            point.x = i32(p) * 0.001f; point.y = i32(p + 4) * 0.001f; point.z = i32(p + 8) * 0.001f;
            point.reflectivity = p[12]; if (type != 0) point.tag = p[13]; return true;
        }
        const bool multiple = type == 5 || type == 8;
        const uint16_t theta = u16(p + (multiple ? 0 : 4)), phi = u16(p + (multiple ? 2 : 6));
        if (theta > 18000 || phi > 36000) return false;
        const uint8_t *depth = p + (multiple ? 4 + echo * 6 : 0);
        const float r = u32(depth) * 0.001f, t = theta * 0.00017453292519943296f, a = phi * 0.00017453292519943296f;
        point.x = r * std::sin(t) * std::cos(a); point.y = r * std::sin(t) * std::sin(a); point.z = r * std::cos(t);
        point.reflectivity = multiple ? depth[4] : p[8];
        if (type != 1) point.tag = multiple ? depth[5] : p[9];
        return true;
    }
}
#endif
