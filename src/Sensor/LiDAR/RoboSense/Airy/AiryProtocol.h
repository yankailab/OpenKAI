#ifndef OPENKAI_AIRY_PROTOCOL_H
#define OPENKAI_AIRY_PROTOCOL_H

// Wire layouts follow the Airy User Guide v1.0 (2025). Firing schedules,
// optical-center dimensions and fields absent from that guide were checked
// against RoboSense's decoder_RSAIRY.hpp (reference only; no SDK dependency).
// The numerical firing tables below are adapted from that BSD-licensed source:
// Copyright (c) 2020 RoboSense. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 3. Neither the names of RoboSense, Suteng Innovation Technology, nor its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

namespace kai
{
namespace airy
{
    constexpr std::size_t kMsopSize = 1248;
    constexpr std::size_t kDifopSize = 1248;
    constexpr std::size_t kImuSize = 51;
    constexpr std::size_t kBlockCount = 8;
    constexpr std::size_t kPointCount = 384;
    constexpr uint32_t kFiringPeriodNs = 111080;
    constexpr double kPi = 3.14159265358979323846;

    inline uint16_t u16(const uint8_t *p) { return (uint16_t(p[0]) << 8) | p[1]; }
    inline uint32_t u32(const uint8_t *p) { return (uint32_t(u16(p)) << 16) | u16(p + 2); }
    inline uint32_t le32(const uint8_t *p)
    { return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24); }
    inline int64_t i32(const uint8_t *p)
    { const uint32_t value = u32(p); return value <= INT32_MAX ? int64_t(value) : int64_t(value) - 4294967296LL; }
    inline float f32(const uint8_t *p)
    { const uint32_t value = u32(p); float result; std::memcpy(&result, &value, sizeof(result)); return result; }
    inline bool fail(std::string *error, const char *message)
    { if (error) *error = message; return false; }

    inline bool timestamp(const uint8_t *p, uint64_t &nanoseconds)
    {
        uint64_t seconds = 0;
        for (std::size_t i = 0; i < 6; ++i) seconds = (seconds << 8) | p[i];
        // The MSOP table says ns, but the appendix, vendor decoder, and captured
        // MSOP/DIFOP/IMU timestamps establish microseconds for this wire field.
        const uint64_t fraction = u32(p + 6);
        if (fraction >= 1000000 || seconds > (UINT64_MAX - fraction * 1000) / 1000000000)
            return false;
        nanoseconds = seconds * 1000000000 + fraction * 1000;
        return true;
    }

    inline std::string ipString(const uint8_t *p)
    { return std::to_string(p[0]) + "." + std::to_string(p[1]) + "." + std::to_string(p[2]) + "." + std::to_string(p[3]); }
    inline std::string hexString(const uint8_t *p, std::size_t size, bool separators = false)
    {
        static const char digits[] = "0123456789ABCDEF";
        std::string result;
        for (std::size_t i = 0; i < size; ++i)
        {
            if (separators && i) result += ':';
            result += digits[p[i] >> 4]; result += digits[p[i] & 15];
        }
        return result;
    }

    struct Calibration
    {
        bool valid = false;
        uint8_t installMode = 0xff; // 0: normal, 1: side, 2: Airy M; 0xff: legacy M schedule.
        uint16_t rpm = 600, fovStart = 0, fovEnd = 36000;
        std::array<int16_t, 96> vertical{}, horizontal{}; // Signed hundredths of a degree.
        std::array<uint16_t, 96> rings{};
        bool operator==(const Calibration &other) const
        {
            return valid == other.valid && installMode == other.installMode && rpm == other.rpm &&
                fovStart == other.fovStart && fovEnd == other.fovEnd &&
                vertical == other.vertical && horizontal == other.horizontal;
        }
        bool operator!=(const Calibration &other) const { return !(*this == other); }
    };

    struct Difop
    {
        Calibration calibration;
        std::string sensorIp, destinationIp, mac, serial;
        std::string mainFirmware, baseFirmware, appFirmware, motorFirmware;
        uint16_t msopPort = 0, difopPort = 0, rpm = 0, motorSpeed = 0;
        uint8_t returnMode = 0, timeMode = 0, timeStatus = 0;
        uint64_t timestampNs = 0;
        double mainVoltage = 0, inputVoltage = 0, baseVoltage = 0, temperature = 0;
        std::array<double, 4> imuQuaternion{}; // Raw DIFOP XYZW, convention not specified by the manual.
        std::array<double, 3> imuTranslation{};
        bool imuCalibrationValid = false;
        std::array<uint8_t, kDifopSize> raw{};
    };

    inline bool decodeDifop(const uint8_t *p, std::size_t size, Difop &result, std::string *error = nullptr)
    {
        static const uint8_t magic[] = {0xa5, 0xff, 0x00, 0x5a, 0x11, 0x11, 0x55, 0x55};
        if (!p || size != kDifopSize) return fail(error, "Airy DIFOP must contain 1248 bytes");
        if (std::memcmp(p, magic, 8) || p[1246] != 0x0f || p[1247] != 0xf0)
            return fail(error, "Invalid Airy DIFOP header or tail");
        Difop decoded;
        decoded.rpm = u16(p + 8);
        decoded.sensorIp = ipString(p + 10); decoded.destinationIp = ipString(p + 14);
        decoded.mac = hexString(p + 18, 6, true);
        decoded.msopPort = u16(p + 24); decoded.difopPort = u16(p + 28);
        decoded.mainFirmware = hexString(p + 40, 5, true);
        decoded.baseFirmware = hexString(p + 45, 5, true);
        decoded.appFirmware = hexString(p + 50, 5, true);
        decoded.motorFirmware = hexString(p + 55, 5, true);
        decoded.serial = hexString(p + 292, 6);
        decoded.returnMode = p[300]; decoded.timeMode = p[301]; decoded.timeStatus = p[302];
        if (!timestamp(p + 303, decoded.timestampNs)) return fail(error, "Invalid Airy DIFOP timestamp");
        decoded.motorSpeed = u16(p + 373);
        decoded.mainVoltage = u16(p + 1044) * 0.01;
        decoded.inputVoltage = u16(p + 1066) * 0.01;
        decoded.baseVoltage = u16(p + 1068) * 0.01;
        decoded.temperature = u16(p + 1080) * 0.01;
        Calibration &calibration = decoded.calibration;
        calibration.installMode = p[289];
        calibration.rpm = decoded.rpm;
        calibration.fovStart = u16(p + 32); calibration.fovEnd = u16(p + 34);
        if (calibration.fovStart > 36000 || calibration.fovEnd > 36000)
            return fail(error, "Invalid Airy field-of-view angles");
        bool nonzero = false;
        for (std::size_t channel = 0; channel < 96; ++channel)
        {
            const uint8_t *v = p + 468 + channel * 3, *h = p + 756 + channel * 3;
            if (v[0] > 1 || h[0] > 1 || u16(v + 1) > 9000 || u16(h + 1) > 18000)
                return fail(error, "Invalid Airy channel calibration");
            calibration.vertical[channel] = static_cast<int16_t>((v[0] ? -1 : 1) * int(u16(v + 1)));
            calibration.horizontal[channel] = static_cast<int16_t>((h[0] ? -1 : 1) * int(u16(h + 1)));
            nonzero |= calibration.vertical[channel] != 0 || calibration.horizontal[channel] != 0;
        }
        calibration.valid = nonzero;
        for (std::size_t channel = 0; channel < 96; ++channel)
            for (std::size_t other = 0; other < 96; ++other)
                calibration.rings[channel] += calibration.vertical[other] < calibration.vertical[channel];
        double norm = 0;
        bool finite = true;
        for (std::size_t i = 0; i < 4; ++i)
        {
            decoded.imuQuaternion[i] = f32(p + 1092 + 4 * i);
            finite &= std::isfinite(decoded.imuQuaternion[i]);
            norm += decoded.imuQuaternion[i] * decoded.imuQuaternion[i];
        }
        for (std::size_t i = 0; i < 3; ++i)
        {
            decoded.imuTranslation[i] = f32(p + 1108 + 4 * i);
            finite &= std::isfinite(decoded.imuTranslation[i]);
        }
        decoded.imuCalibrationValid = finite && norm > 0.5 && norm < 1.5;
        std::memcpy(decoded.raw.data(), p, size);
        result = decoded;
        if (error) error->clear();
        return true;
    }

    struct Point
    {
        float x = 0, y = 0, z = 0, range = 0;
        uint8_t intensity = 0, feature = 0, channel = 0, block = 0;
        uint16_t ring = 0, blockAzimuth = 0;
        uint32_t offsetNs = 0;
        uint64_t timestampNs = 0;
    };
    struct Msop
    {
        uint32_t sequence = 0;
        bool sequenceValid = false;
        uint64_t timestampNs = 0;
        uint8_t model = 0, returnMode = 0, lidarType = 0;
        bool dual = false;
        float temperature = 0, topTemperature = 0;
        std::array<Point, kPointCount> points{};
        std::array<uint16_t, kBlockCount> azimuths{};
        std::array<uint32_t, kBlockCount> blockOffsetsNs{};
    };

    inline bool validMsopHeader(const uint8_t *p, std::size_t size)
    {
        static const uint8_t magic[] = {0x55, 0xaa, 0x05, 0x5a};
        return p && size == kMsopSize && std::memcmp(p, magic, 4) == 0 &&
            (p[31] == 0x10 || p[31] == 0x31); // 0x31 verified on the connected Airy.
    }

    inline float temperature(const uint8_t *p)
    {
        const unsigned magnitude = (unsigned(p[1] & 0x7f) << 5) | (p[0] >> 3);
        return (p[1] & 0x80 ? -1.0f : 1.0f) * float(magnitude) * 0.0625f;
    }

    inline uint32_t firingOffsetNs(uint8_t model, uint8_t installMode, unsigned channel)
    {
        // One simultaneous firing group contains four channels in 48-beam mode,
        // eight in 96-beam mode. Table values are integral nanoseconds.
        static const uint32_t standard[] = {0,7616,16184,24752,33320,42840,52360,61880,71400,79968,88536,97104};
        static const uint32_t normal[] = {0,5712,12376,19040,25704,33320,41888,50456,59024,70448,81872,93296};
        static const uint32_t side[] = {0,11424,22848,34272,45696,54264,62832,71400,79016,85680,92344,99008};
        if (model == 1) return standard[channel / 4];
        const unsigned group = channel / 8;
        return installMode == 0 ? normal[group] : installMode == 1 ? side[group] : standard[group];
    }

    inline bool decodeMsop(const uint8_t *p, std::size_t size, const Calibration &calibration,
                           Msop &result, std::string *error = nullptr)
    {
        if (!validMsopHeader(p, size)) return fail(error, "Invalid Airy MSOP size, header or sensor type");
        if (p[16] != 0) return fail(error, "Airy packet is not point-cloud data");
        if (p[32] != 1 && p[32] != 2) return fail(error, "Unsupported Airy channel model (expected 48 or 96 beams)");
        if (p[1230] != 0x00 || p[1231] != 0xff) return fail(error, "Invalid Airy MSOP tail");
        if (!calibration.valid) return fail(error, "Airy channel calibration is not available; wait for DIFOP");
        if (calibration.installMode > 2 && calibration.installMode != 0xff)
            return fail(error, "Unsupported Airy installation mode");
        Msop decoded;
        decoded.sequence = u32(p + 12);
        decoded.sequenceValid = decoded.sequence <= 65535;
        decoded.model = p[32]; decoded.lidarType = p[31]; decoded.returnMode = p[17];
        decoded.dual = decoded.returnMode == 3;
        decoded.temperature = temperature(p + 38); decoded.topTemperature = temperature(p + 40);
        if (!timestamp(p + 20, decoded.timestampNs)) return fail(error, "Invalid Airy MSOP timestamp");
        for (std::size_t block = 0; block < kBlockCount; ++block)
        {
            const uint8_t *b = p + 42 + block * 148;
            if (b[0] != 0xff || b[1] != 0xee || u16(b + 2) >= 36000)
                return fail(error, "Invalid Airy block marker or azimuth");
            decoded.azimuths[block] = u16(b + 2);
        }

        const unsigned channelBlocks = decoded.model == 2 ? 2 : 1;
        const unsigned groupBlocks = channelBlocks * (decoded.dual ? 2 : 1);
        const double rotations = (calibration.rpm ? calibration.rpm : 600) / 60.0;
        const int nominalStep = static_cast<int>(std::lround(36000.0 * rotations * kFiringPeriodNs * 1e-9));
        unsigned fovSpan = calibration.fovEnd > calibration.fovStart ? calibration.fovEnd - calibration.fovStart :
            calibration.fovEnd + 36000 - calibration.fovStart;
        if (fovSpan == 0) fovSpan = 36000;
        const double blindNs = (36000 - fovSpan) * 1e9 / (36000.0 * rotations);
        const double radialCenter = std::sqrt(0.0075 * 0.0075 + 0.00664 * 0.00664);
        uint64_t groupOffset = 0;
        for (unsigned group = 0; group < kBlockCount; group += groupBlocks)
        {
            int step = nominalStep;
            uint64_t elapsed = kFiringPeriodNs;
            if (group + groupBlocks < kBlockCount)
            {
                step = (int(decoded.azimuths[group + groupBlocks]) - decoded.azimuths[group] + 36000) % 36000;
                // Cropped FOV packets can cross the skipped angular sector.
                // The next firing resumes after the configured blind interval.
                if (step > 100)
                {
                    step = nominalStep;
                    if (fovSpan < 36000) elapsed = static_cast<uint64_t>(std::llround(blindNs));
                }
            }
            for (unsigned local = 0; local < groupBlocks; ++local)
            {
                const unsigned block = group + local;
                if (groupOffset > UINT32_MAX) return fail(error, "Airy block time offset overflow");
                decoded.blockOffsetsNs[block] = static_cast<uint32_t>(groupOffset);
                for (unsigned lane = 0; lane < 48; ++lane)
                {
                    const unsigned channel = lane + (decoded.model == 2 && block % 2 ? 48 : 0);
                    const uint32_t firing = firingOffsetNs(decoded.model, calibration.installMode, channel);
                    const uint64_t offset = groupOffset + firing;
                    if (offset > UINT32_MAX || decoded.timestampNs > UINT64_MAX - offset)
                        return fail(error, "Airy point timestamp overflow");
                    const uint8_t *raw = p + 42 + block * 148 + 4 + lane * 3;
                    Point &point = decoded.points[block * 48 + lane];
                    const uint16_t packedDistance = u16(raw);
                    point.range = (packedDistance & 0x3fff) * 0.005f;
                    point.intensity = raw[2]; point.feature = uint8_t(packedDistance >> 14);
                    point.channel = static_cast<uint8_t>(channel); point.block = static_cast<uint8_t>(block);
                    point.ring = calibration.rings[channel]; point.blockAzimuth = decoded.azimuths[block];
                    point.offsetNs = static_cast<uint32_t>(offset);
                    point.timestampNs = decoded.timestampNs + offset;
                    // Encoder angles increase clockwise. Keep the sensor's
                    // right-handed XYZ frame, including its optical-center offset.
                    const int encoder = decoded.azimuths[block] + static_cast<int>(double(step) * firing / kFiringPeriodNs);
                    const double azimuth = (encoder + calibration.horizontal[channel]) * kPi / 18000.0;
                    const double elevation = calibration.vertical[channel] * kPi / 18000.0;
                    const double encoderRadians = encoder * kPi / 18000.0;
                    point.x = float(point.range * std::cos(elevation) * std::cos(azimuth) + radialCenter * std::cos(encoderRadians));
                    point.y = float(-point.range * std::cos(elevation) * std::sin(azimuth) - radialCenter * std::sin(encoderRadians));
                    point.z = float(point.range * std::sin(elevation) + 0.04532);
                }
            }
            groupOffset += elapsed;
        }
        result = decoded;
        if (error) error->clear();
        return true;
    }

    struct Imu
    {
        uint64_t timestampNs = 0;
        uint32_t sequence = 0;
        std::array<double, 3> acceleration{}, angularVelocity{}; // m/s^2 and rad/s, native IMU axes.
        uint8_t outputRate = 0, accelRange = 0, gyroRange = 0;
        int32_t temperatureRaw = 0;
    };

    inline bool decodeImu(const uint8_t *p, std::size_t size, Imu &result, std::string *error = nullptr)
    {
        static const uint8_t magic[] = {0xaa, 0x55, 0x5a, 0x05};
        if (!p || size != kImuSize) return fail(error, "Airy IMU must contain 51 bytes");
        if (std::memcmp(p, magic, 4) || p[49] != 0xf0 || p[50] != 0x0f)
            return fail(error, "Invalid Airy IMU header or tail");
        if (p[42] > 3 || p[43] > 3 || p[44] > 3) return fail(error, "Invalid Airy IMU rate or full-scale code");
        Imu decoded;
        if (!timestamp(p + 4, decoded.timestampNs)) return fail(error, "Invalid Airy IMU timestamp");
        decoded.outputRate = p[42]; decoded.accelRange = p[43]; decoded.gyroRange = p[44];
        decoded.sequence = le32(p + 45); // Confirmed 200 Hz counter in live packets.
        decoded.temperatureRaw = static_cast<int32_t>(i32(p + 38));
        const double accelerationScale = double(2u << decoded.accelRange) * 9.80665 / 32768.0;
        const double angularScale = double(250u << decoded.gyroRange) * kPi / (32768.0 * 180.0);
        for (unsigned axis = 0; axis < 3; ++axis)
        {
            decoded.acceleration[axis] = i32(p + 14 + axis * 4) * accelerationScale;
            decoded.angularVelocity[axis] = i32(p + 26 + axis * 4) * angularScale;
        }
        result = decoded;
        if (error) error->clear();
        return true;
    }
}
}
#endif
