#ifndef OPENKAI_LIVOX2_PROTOCOL_H
#define OPENKAI_LIVOX2_PROTOCOL_H

#include "../../Dependencies/CRC.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace kai::livox2
{
    // Livox SDK2 wire fields are little-endian and frequently unaligned.
    inline uint16_t u16(const uint8_t *p) { return uint16_t(p[0]) | (uint16_t(p[1]) << 8); }
    inline uint32_t u32(const uint8_t *p) { return uint32_t(u16(p)) | (uint32_t(u16(p + 2)) << 16); }
    inline uint64_t u64(const uint8_t *p) { return uint64_t(u32(p)) | (uint64_t(u32(p + 4)) << 32); }

    inline bool validCommand(const uint8_t *p, size_t size, bool crc = true)
    {
        if (!p || size < 24 || size > 1368 || p[0] != 0xaa || p[1] != 0 || u16(p + 2) != size ||
            p[10] > 1 || p[11] != 1) return false;
        if (!crc) return true;
        return CRC::Calculate(p, 18, CRC::CRC_16_CCITTFALSE()) == u16(p + 18) &&
            (size == 24 ? 0 : CRC::Calculate(p + 24, size - 24, CRC::CRC_32())) == u32(p + 20);
    }

    inline size_t pointSize(uint8_t type)
    {
        switch (type) { case 0: return 24; case 1: return 14; case 2: return 8; case 3: return 10; default: return 0; }
    }

    inline bool validData(const uint8_t *p, size_t size, bool crc = true)
    {
        if (!p || size < 36 || size > 1380 || p[0] != 0 || u16(p + 1) != size || p[11] > 2) return false;
        const size_t stride = pointSize(p[10]), count = u16(p + 5);
        if (!stride || !count || count > 1344 / stride || size != 36 + count * stride || !u64(p + 28)) return false;
        if (u64(p + 28) > UINT64_MAX - uint64_t(u16(p + 3)) * 100) return false;
        // Mid-360 firmware omits the point-cloud checksum (zero), but supplies
        // IMU checksums. Check every checksum supplied by the device.
        const uint32_t checksum = u32(p + 24);
        return !crc || (!checksum && p[10] != 0) ||
            CRC::Calculate(p + 28, size - 28, CRC::CRC_32()) == checksum;
    }

    inline uint64_t pointTime(uint64_t first, uint16_t interval, uint16_t count, size_t index)
    {
        // time_interval spans the FIRST through LAST point, in 0.1 microseconds.
        return first + (count > 1 ? uint64_t(interval) * 100 * index / (count - 1) : 0);
    }
}
#endif
