#include "SLAM/_SLAMbase.h"
#include <iostream>
#include <stdexcept>

using namespace kai;

namespace
{
    void require(bool condition, const char *message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    class InputProbe : public _SLAMbase
    {
    public:
        explicit InputProbe(IMUstream &stream) { m_pIMUin = &stream; }
        using _SLAMbase::readIMU;
    };

    constexpr uint64_t ms = NSEC_MSEC;
    const Vector3f angular(0.1f, -0.2f, 0.3f);

    void expect(InputProbe &probe, uint64_t expectedStamp, double expectedAcc)
    {
        Vector3d acc, gyro;
        uint64_t stamp = 0;
        require(probe.readIMU(acc, gyro, stamp), "Missing synchronized IMU sample");
        require(stamp == expectedStamp, "IMU sample lost its gyro capture timestamp");
        require(acc.isApprox(Vector3d(expectedAcc, 0, 9.81), 1e-6), "Wrong acceleration interpolation");
        require(gyro.isApprox(angular.cast<double>(), 1e-8), "Gyro vector was changed");
    }

    void empty(InputProbe &probe)
    {
        Vector3d acc, gyro;
        uint64_t stamp = 0;
        require(!probe.readIMU(acc, gyro, stamp), "Unexpected IMU sample or extrapolation");
    }

    void acceleration(IMUstream &stream, double value, uint64_t stamp)
    {
        stream.addAcc(Vector3f(value, 0, 9.81f), stamp);
    }

    void testMixedRatesAndIndependentReaders()
    {
        IMUstream stream;
        InputProbe first(stream), second(stream);
        for (uint64_t t = 10; t <= 40; t += 10) acceleration(stream, t, t * ms);
        for (uint64_t t = 10; t <= 40; t += 5) stream.addGyro(angular, t * ms);
        for (uint64_t t = 10; t <= 40; t += 5) expect(first, t * ms, t);
        empty(first);
        empty(first);
        for (uint64_t t = 10; t <= 40; t += 5) expect(second, t * ms, t);
        empty(second);
    }

    void testD455Rates()
    {
        IMUstream stream;
        InputProbe probe(stream);
        for (uint64_t t = 10; t <= 50; t += 4) acceleration(stream, t, t * ms);
        for (uint64_t t = 10; t <= 50; t += 5) stream.addGyro(angular, t * ms);
        for (uint64_t t = 10; t <= 50; t += 5) expect(probe, t * ms, t);
        empty(probe);
    }

    void testDelayedAccelerationAndBatchBoundary()
    {
        IMUstream stream;
        InputProbe probe(stream);
        acceleration(stream, 10, 10 * ms);
        stream.addGyro(angular, 10 * ms);
        stream.addGyro(angular, 15 * ms);
        expect(probe, 10 * ms, 10);
        empty(probe);
        empty(probe);
        acceleration(stream, 20, 20 * ms);
        expect(probe, 15 * ms, 15);
        empty(probe);
        stream.addGyro(angular, 20 * ms);
        expect(probe, 20 * ms, 20);
        empty(probe);
        // Same-timestamp channel delivery must not be gated on DataObject time.
        stream.addGyro(angular, 25 * ms);
        empty(probe);
        acceleration(stream, 25, 25 * ms);
        expect(probe, 25 * ms, 25);
        empty(probe);
        stream.addGyro(angular, 25 * ms);
        acceleration(stream, 25, 25 * ms);
        empty(probe);
    }

    void testMissingHistoryAndLongOutage()
    {
        IMUstream stream;
        InputProbe probe(stream);
        acceleration(stream, 10, 10 * ms);
        acceleration(stream, 20, 20 * ms);
        stream.addGyro(angular, 5 * ms);
        stream.addGyro(angular, 15 * ms);
        expect(probe, 15 * ms, 15);
        empty(probe);
        stream.addGyro(angular, 100 * ms);
        acceleration(stream, 220, 220 * ms);
        empty(probe);
        stream.addGyro(angular, 220 * ms);
        expect(probe, 220 * ms, 220);
        empty(probe);
    }

    void testClockReset(bool resetGyro, bool pendingGyro)
    {
        IMUstream stream;
        InputProbe probe(stream);
        acceleration(stream, 10, 10 * ms);
        stream.addGyro(angular, 10 * ms);
        expect(probe, 10 * ms, 10);
        empty(probe);
        if (pendingGyro)
        {
            stream.addGyro(angular, 30 * ms);
            empty(probe);
        }
        if (resetGyro) stream.addGyro(angular, pendingGyro ? 20 * ms : 5 * ms);
        else acceleration(stream, 5, 5 * ms);
        bool rejected = false;
        try { empty(probe); }
        catch (const std::runtime_error &error)
        {
            rejected = string(error.what()).find("clock reset") != string::npos;
        }
        require(rejected, "A capture clock reset was not rejected");
    }
}

int main(int argc, char **argv)
{
    google::InitGoogleLogging(argv[0]);
    try
    {
        testMixedRatesAndIndependentReaders();
        testD455Rates();
        testDelayedAccelerationAndBatchBoundary();
        testMissingHistoryAndLongOutage();
        testClockReset(true, false);
        testClockReset(false, false);
        testClockReset(true, true);
        std::cout << "PASS: unequal IMU rates, interpolation, delayed channels, independent readers, gaps and clock resets\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
