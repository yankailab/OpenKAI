#ifndef OPENKAI_REALSENSE_IMU_H
#define OPENKAI_REALSENSE_IMU_H

#include "../../Dependencies/SensorFusion/SensorFusion.h"
#include <Eigen/Geometry>
#include <cstdint>
#include <mutex>
#include <optional>

namespace kai::realsense
{
    // A viewer-only snapshot. The original samples still go independently to
    // IMUstream; preview reads and fusion never consume the SLAM input queues.
    class IMUPreview
    {
    public:
        struct Snapshot
        {
            Eigen::Vector3f gyro = Eigen::Vector3f::Zero();
            Eigen::Vector3f acc = Eigen::Vector3f::Zero();
            uint64_t tGyro = 0;
            uint64_t tAcc = 0;
            uint64_t tFusion = 0;
            Eigen::Quaterniond quaternion = Eigen::Quaterniond::Identity();
            Eigen::Vector3d rpy = Eigen::Vector3d::Zero();
            bool orientationValid = false;
        };

        void reset()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_snapshot = Snapshot{};
            resetFusion();
        }

        void addAcc(const Eigen::Vector3f &acc, uint64_t captureNs)
        {
            if (!captureNs || !acc.allFinite())
                return;
            std::lock_guard<std::mutex> lock(m_mutex);
            if (captureNs == m_snapshot.tAcc)
                return;
            if (discontinuity(captureNs, m_snapshot.tAcc))
                resetFusion();
            m_snapshot.acc = acc;
            m_snapshot.tAcc = captureNs;
        }

        void addGyro(const Eigen::Vector3f &gyro, uint64_t captureNs)
        {
            if (!captureNs || !gyro.allFinite())
                return;
            std::lock_guard<std::mutex> lock(m_mutex);
            if (captureNs == m_snapshot.tGyro)
                return;
            if (discontinuity(captureNs, m_snapshot.tGyro))
                resetFusion();
            m_snapshot.gyro = gyro;
            m_snapshot.tGyro = captureNs;

            const uint64_t skew = captureNs >= m_snapshot.tAcc
                ? captureNs - m_snapshot.tAcc : m_snapshot.tAcc - captureNs;
            const float accNorm2 = m_snapshot.acc.squaredNorm();
            if (!m_snapshot.tAcc || skew > kAccToleranceNs ||
                !std::isfinite(accNorm2) || accNorm2 < 1.0e-12f)
            {
                resetFusion();
                return;
            }

            const uint64_t previous = m_snapshot.tFusion;
            m_snapshot.tFusion = captureNs;
            if (!previous)
                return;

            // RealSense gyro is radians/s and acceleration is m/s^2. Capture
            // timestamps determine dt; the preview/websocket rate does not.
            const float dt = static_cast<float>(captureNs - previous) * 1.0e-9f;
            const auto &acc = m_snapshot.acc;
            m_fusion->MahonyUpdate(gyro.x(), gyro.y(), gyro.z(),
                                  acc.x(), acc.y(), acc.z(), dt);
            const float *q = m_fusion->getQuat(); // SF order: w, x, y, z.
            Eigen::Quaterniond orientation(q[0], q[1], q[2], q[3]);
            if (!orientation.coeffs().allFinite() || orientation.norm() < 1.0e-12)
            {
                resetFusion();
                return;
            }
            m_snapshot.quaternion = orientation.normalized();
            // ZYX decomposition returns yaw/pitch/roll; publish roll/pitch/yaw.
            m_snapshot.rpy = m_snapshot.quaternion.toRotationMatrix()
                .canonicalEulerAngles(2, 1, 0).reverse();
            m_snapshot.orientationValid = m_snapshot.rpy.allFinite();
            if (!m_snapshot.orientationValid)
                resetFusion();
        }

        Snapshot snapshot() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_snapshot;
        }

    private:
        static constexpr uint64_t kAccToleranceNs = 50000000;
        static constexpr uint64_t kMaxGapNs = 500000000;

        static bool discontinuity(uint64_t current, uint64_t previous)
        {
            return previous && (current < previous || current - previous > kMaxGapNs);
        }

        // Caller owns m_mutex. Reconstruct SF instead of copying its lazily
        // initialized angle cache and scratch storage from a temporary.
        void resetFusion()
        {
            m_fusion.emplace();
            m_snapshot.tFusion = 0;
            m_snapshot.quaternion.setIdentity();
            m_snapshot.rpy.setZero();
            m_snapshot.orientationValid = false;
        }

        mutable std::mutex m_mutex;
        Snapshot m_snapshot;
        std::optional<SF> m_fusion{std::in_place};
    };
}

#endif
