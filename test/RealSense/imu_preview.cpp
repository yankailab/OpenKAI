#include "../../src/Vision/RGBD/RealSenseIMU.h"
#include <atomic>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

using kai::realsense::IMUPreview;
using Eigen::Vector3f;

namespace
{
    constexpr uint64_t second = 1000000000;
    constexpr uint64_t step = 5000000;
    const Vector3f gravity(0, 0, 9.80665f);

    void require(bool result, const char *message)
    {
        if (!result)
            throw std::runtime_error(message);
    }

    void stationaryAndRotation()
    {
        IMUPreview preview;
        const auto empty = preview.snapshot();
        require(!empty.tGyro && !empty.tAcc && !empty.tFusion && !empty.orientationValid,
                "Initial snapshot has no measurements or orientation");
        require(empty.quaternion.isApprox(Eigen::Quaterniond::Identity()),
                "Initial orientation is identity");

        for (uint64_t t = second; t <= second * 2; t += step)
        {
            preview.addAcc(gravity, t);
            preview.addGyro(Vector3f::Zero(), t);
        }
        auto value = preview.snapshot();
        require(value.orientationValid && value.rpy.norm() < 1.0e-6,
                "Stationary samples produce a valid identity orientation");
        require(value.tFusion == second * 2, "Fusion uses capture timestamps");

        // A 90 degree/s yaw about gravity should turn through pi/2 in one
        // second. This also checks the gyro input is interpreted as radians/s.
        const Vector3f yawRate(0, 0, static_cast<float>(std::acos(-1.0) / 2.0));
        for (uint64_t t = second * 2 + step; t <= second * 3; t += step)
        {
            // Hold each acceleration sample across 4 gyro frames (50 Hz).
            if ((t - second * 2) % (step * 4) == 0)
                preview.addAcc(gravity, t);
            preview.addGyro(yawRate, t);
        }
        value = preview.snapshot();
        require(value.orientationValid, "Held acceleration supports faster gyro updates");
        require(std::abs(value.rpy.z() - std::acos(-1.0) / 2.0) < 0.001,
                "Known yaw integrates using actual capture dt");
        require(std::abs(value.quaternion.norm() - 1.0) < 1.0e-12,
                "Published quaternion is normalized");

        const auto previous = value;
        preview.addGyro(Vector3f(10, 10, 10), value.tGyro);
        preview.addAcc(Vector3f(20, 20, 20), value.tAcc);
        value = preview.snapshot();
        require(value.quaternion.isApprox(previous.quaternion) && value.gyro == previous.gyro &&
                value.acc == previous.acc, "Duplicate timestamps are ignored");
    }

    void invalidAndStaleSamples()
    {
        IMUPreview preview;
        const float nan = std::numeric_limits<float>::quiet_NaN();
        preview.addAcc(Vector3f(nan, 0, 0), second);
        preview.addGyro(Vector3f(0, nan, 0), second);
        preview.addAcc(gravity, 0);
        preview.addGyro(Vector3f::Zero(), 0);
        auto value = preview.snapshot();
        require(!value.tGyro && !value.tAcc, "Invalid samples do not change the snapshot");

        preview.addGyro(Vector3f(0, 0, 1), second);
        preview.addGyro(Vector3f(0, 0, 1), second + step);
        require(!preview.snapshot().orientationValid, "Gyro alone cannot initialize fusion");
        preview.addAcc(gravity, second + step);
        preview.addGyro(Vector3f::Zero(), second + step * 2);
        require(!preview.snapshot().orientationValid, "First usable pair seeds the clock");
        preview.addGyro(Vector3f::Zero(), second + step * 3);
        require(preview.snapshot().orientationValid, "Next usable pair produces orientation");
        preview.addGyro(Vector3f::Zero(), second + step * 12);
        require(!preview.snapshot().orientationValid, "Stale acceleration invalidates orientation");
        preview.addAcc(Vector3f::Zero(), second + step * 13);
        preview.addGyro(Vector3f::Zero(), second + step * 13);
        require(!preview.snapshot().orientationValid, "Zero acceleration cannot initialize orientation");
    }

    void clockDiscontinuities()
    {
        IMUPreview preview;
        auto pair = [&](uint64_t t) {
            preview.addAcc(gravity, t);
            preview.addGyro(Vector3f(0, 0, 1), t);
        };
        pair(second);
        pair(second + step);
        require(preview.snapshot().orientationValid, "Setup valid orientation");

        // Either sensor's clock reset must clear the previous integration.
        preview.addAcc(gravity, second / 2);
        require(!preview.snapshot().orientationValid && !preview.snapshot().tFusion,
                "Acceleration timestamp regression clears fusion");
        preview.addGyro(Vector3f(0, 0, 1), second / 2);
        require(!preview.snapshot().orientationValid, "Gyro regression seeds a new baseline");
        pair(second / 2 + step);
        require(preview.snapshot().orientationValid, "Fusion resumes in the new epoch");

        preview.addGyro(Vector3f(0, 0, 1), second / 2);
        require(!preview.snapshot().orientationValid,
                "Gyro regression resets an otherwise valid orientation");
        pair(second / 2 + step * 2);
        require(preview.snapshot().orientationValid, "Fresh gyro recovers after regression");

        pair(second * 3);
        require(!preview.snapshot().orientationValid, "Long gap cannot integrate old angular velocity");
        pair(second * 3 + step);
        require(preview.snapshot().orientationValid, "Fusion resumes after a gap");
        require(preview.snapshot().rpy.norm() < 0.01, "Gap resets the previous orientation");

        preview.reset();
        const auto value = preview.snapshot();
        require(!value.tGyro && !value.tAcc && !value.tFusion && !value.orientationValid &&
                value.gyro.isZero() && value.acc.isZero() && value.rpy.isZero(),
                "Camera close reset clears measurements and fusion");
    }

    void concurrentSnapshots()
    {
        IMUPreview preview;
        std::atomic<bool> done{false};
        std::atomic<bool> coherent{true};
        std::thread reader([&] {
            while (!done.load())
            {
                const auto value = preview.snapshot();
                if (value.tFusion > value.tGyro || !value.quaternion.coeffs().allFinite() ||
                    std::abs(value.quaternion.norm() - 1.0) > 1.0e-10 ||
                    (value.orientationValid && (!value.tAcc || !value.tGyro || !value.tFusion)))
                    coherent = false;
            }
        });
        std::thread accel([&] {
            for (uint64_t i = 0; i < 2000; ++i)
                preview.addAcc(gravity, second + i * step);
        });
        std::thread gyro([&] {
            for (uint64_t i = 0; i < 2000; ++i)
                preview.addGyro(Vector3f(0, 0, 1), second + i * step);
        });
        accel.join();
        gyro.join();
        done = true;
        reader.join();
        require(coherent, "Concurrent producers and snapshot reader see coherent state");
    }
}

int main()
{
    try
    {
        stationaryAndRotation();
        invalidAndStaleSamples();
        clockDiscontinuities();
        concurrentSnapshots();
        std::cout << "RealSense IMU preview: fusion, timestamp guards, reset and snapshots passed\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
