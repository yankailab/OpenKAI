#include "../../src/SLAM/_SLAMbase.h"
#include "../../src/UI/_Console.h"
#include <cassert>
#include <stdexcept>

namespace kai
{
	void _Console::addMsg(const string &, int)
	{
	}

	void _Console::addMsg(const string &, int, int, int)
	{
	}

	InstanceMgr::InstanceMgr() = default;
	InstanceMgr::~InstanceMgr() = default;

	void *InstanceMgr::findDataStream(const string &)
	{
		return nullptr;
	}

	void *InstanceMgr::findModule(const string &)
	{
		return nullptr;
	}
}

namespace
{
	class TestSLAM : public kai::_SLAMbase
	{
	public:
		TestSLAM(kai::PCLframe &points, kai::IMUstream &imu)
		{
			m_pPCL = &points;
			m_pIMU = &imu;
		}

		using _SLAMbase::readPointCloud;
		using _SLAMbase::readIMU;

		bool check(void) override
		{
			return m_pPCL != nullptr;
		}

	protected:
		bool startSLAM(void) override
		{
			return true;
		}
	};

	kai::GEOMETRY_POINT point(float x, uint64_t timestamp)
	{
		kai::GEOMETRY_POINT result;
		result.m_vP = Eigen::Vector3f(x, 2, 3);
		result.m_tStamp = timestamp;
		return result;
	}

	void addIMU(kai::IMUstream &imu, uint64_t timestamp)
	{
		imu.set(kai::IMUstream::Type::Gyro, Eigen::Vector3f(1, 2, 3), timestamp);
		imu.set(kai::IMUstream::Type::Acc, Eigen::Vector3f(4, 5, 6), timestamp);
	}

	void testPointClouds()
	{
		kai::PCLframe points;
		kai::IMUstream imu;
		TestSLAM first(points, imu);
		TestSLAM second(points, imu);
		assert(!first.readPointCloud());
		points.set({point(1, 100)}, 100);
		const auto held = first.readPointCloud();
		assert(held == points.get());
		assert(second.readPointCloud() == held);
		assert(!first.readPointCloud());
		assert(!second.readPointCloud());

		points.set({point(7, 200)}, 200);
		assert(first.readPointCloud()->m_vPoints.front().m_vP.x() == 7);
		assert(held->m_vPoints.front().m_vP.x() == 1);
		assert(held->m_tStamp == 100);
		// Empty publications can use a host clock while captures use a device
		// clock. Consume their revision without advancing the estimator clock.
		points.set({}, kai::getTns());
		assert(!first.readPointCloud());
		assert(!first.readPointCloud());
		points.set({}, 50);
		assert(!first.readPointCloud());

		// The data stream revision changes, but the estimator requires increasing
		// capture times. A duplicate capture time must not be integrated twice.
		points.set({point(9, 200)}, 200);
		assert(!first.readPointCloud());
		points.set({point(10, 301)}, 301);
		assert(first.readPointCloud()->m_vPoints.front().m_vP.x() == 10);
		points.set({point(11, 50)}, 50);
		bool resetDetected = false;
		try
		{
			first.readPointCloud();
		}
		catch (const std::runtime_error &error)
		{
			resetDetected = std::string(error.what()).find("clock reset") != std::string::npos;
		}
		assert(resetDetected);
		first.reset();
		assert(first.readPointCloud() == points.get());
		assert(!first.readPointCloud());
	}

	void testIMUreaders()
	{
		kai::PCLframe points;
		kai::IMUstream imu;
		TestSLAM first(points, imu);
		TestSLAM second(points, imu);
		Eigen::Vector3d acc;
		Eigen::Vector3d gyro;
		uint64_t timestamp = 0;
		assert(!first.readIMU(acc, gyro, timestamp));
		addIMU(imu, 1000);
		addIMU(imu, 2000);
		for (TestSLAM *reader : {&first, &second})
		{
			assert(reader->readIMU(acc, gyro, timestamp) && timestamp == 1000);
			assert(acc == Eigen::Vector3d(4, 5, 6));
			assert(gyro == Eigen::Vector3d(1, 2, 3));
			assert(reader->readIMU(acc, gyro, timestamp) && timestamp == 2000);
			assert(!reader->readIMU(acc, gyro, timestamp));
		}
		addIMU(imu, 2000);
		addIMU(imu, 3000);
		assert(first.readIMU(acc, gyro, timestamp) && timestamp == 3000);
		assert(!first.readIMU(acc, gyro, timestamp));
		addIMU(imu, 1500);
		bool resetDetected = false;
		try
		{
			first.readIMU(acc, gyro, timestamp);
		}
		catch (const std::runtime_error &error)
		{
			resetDetected = std::string(error.what()).find("clock reset") != std::string::npos;
		}
		assert(resetDetected);
		first.reset();
		assert(first.readIMU(acc, gyro, timestamp) && timestamp == 1500);
		assert(!first.readIMU(acc, gyro, timestamp));
	}

	void testIMUpairing()
	{
		kai::PCLframe points;
		kai::IMUstream imu;
		TestSLAM slam(points, imu);
		const Vector3f value(1, 2, 3);
		Vector3d acc;
		Vector3d gyro;
		uint64_t timestamp = 0;
		imu.set(kai::IMUstream::Type::Gyro, value, 10 * NSEC_MSEC);
		assert(!slam.readIMU(acc, gyro, timestamp));
		imu.set(kai::IMUstream::Type::Acc, value, 15 * NSEC_MSEC);
		assert(slam.readIMU(acc, gyro, timestamp) && timestamp == 15 * NSEC_MSEC);
		assert(!slam.readIMU(acc, gyro, timestamp));

		// Skip samples outside the tolerance and retain unmatched samples for
		// the next snapshot when the other sensor has not arrived yet.
		imu.set(kai::IMUstream::Type::Gyro, value, 20 * NSEC_MSEC);
		imu.set(kai::IMUstream::Type::Acc, value, 30 * NSEC_MSEC);
		assert(!slam.readIMU(acc, gyro, timestamp));
		imu.set(kai::IMUstream::Type::Gyro, value, 32 * NSEC_MSEC);
		assert(slam.readIMU(acc, gyro, timestamp) && timestamp == 32 * NSEC_MSEC);
		assert(!slam.readIMU(acc, gyro, timestamp));
		imu.set(kai::IMUstream::Type::Acc, value, 40 * NSEC_MSEC);
		imu.set(kai::IMUstream::Type::Gyro, value, 50 * NSEC_MSEC);
		assert(!slam.readIMU(acc, gyro, timestamp));
		imu.set(kai::IMUstream::Type::Acc, value, 51 * NSEC_MSEC);
		assert(slam.readIMU(acc, gyro, timestamp) && timestamp == 51 * NSEC_MSEC);
		assert(!slam.readIMU(acc, gyro, timestamp));
	}

	void testIMUbatchAndStaggeredReset()
	{
		kai::PCLframe points;
		kai::IMUstream imu;
		TestSLAM slam(points, imu);
		Vector3d acc;
		Vector3d gyro;
		uint64_t timestamp = 0;
		addIMU(imu, 50 * NSEC_MSEC);
		addIMU(imu, 100 * NSEC_MSEC);
		assert(slam.readIMU(acc, gyro, timestamp) && timestamp == 50 * NSEC_MSEC);
		addIMU(imu, 150 * NSEC_MSEC);
		assert(slam.readIMU(acc, gyro, timestamp) && timestamp == 100 * NSEC_MSEC);
		assert(!slam.readIMU(acc, gyro, timestamp));
		assert(slam.readIMU(acc, gyro, timestamp) && timestamp == 150 * NSEC_MSEC);
		assert(!slam.readIMU(acc, gyro, timestamp));
		imu.set(kai::IMUstream::Type::Gyro, Vector3f::Zero(), 200 * NSEC_MSEC);
		assert(!slam.readIMU(acc, gyro, timestamp));
		imu.set(kai::IMUstream::Type::Acc, Vector3f::Zero(), 10 * NSEC_MSEC);
		bool resetDetected = false;
		try
		{
			slam.readIMU(acc, gyro, timestamp);
		}
		catch (const std::runtime_error &error)
		{
			resetDetected = std::string(error.what()).find("clock reset") != std::string::npos;
		}
		assert(resetDetected);
	}

	void testSessionCursors()
	{
		kai::PCLframe points;
		kai::IMUstream imu;
		TestSLAM slam(points, imu);
		points.set({point(1, 100)}, 100);
		addIMU(imu, 100);
		Eigen::Vector3d acc;
		Eigen::Vector3d gyro;
		uint64_t timestamp = 0;
		assert(slam.startTracking());
		assert(slam.bTracking());
		assert(slam.readPointCloud());
		assert(slam.readIMU(acc, gyro, timestamp));
		assert(slam.startTracking());
		assert(!slam.readPointCloud());
		assert(!slam.readIMU(acc, gyro, timestamp));
		slam.stopTracking();
		assert(!slam.bTracking());
		assert(slam.startTracking());
		assert(slam.readPointCloud());
		assert(slam.readIMU(acc, gyro, timestamp));
		slam.setPos(Eigen::Vector3d(1, 2, 3));
		slam.reset();
		assert(!slam.bTracking());
		assert(slam.getPos().isZero());
		assert(slam.getOrientation().isApprox(Eigen::Quaterniond::Identity()));
		assert(slam.readPointCloud());
		assert(slam.readIMU(acc, gyro, timestamp));
	}
}

int main()
{
	testPointClouds();
	testIMUreaders();
	testIMUpairing();
	testIMUbatchAndStaggeredReset();
	testSessionCursors();
	std::cout << "PASS SLAM streams: immutable inputs, independent readers, capture-time checks, reset/restart cursors\n";
	return 0;
}
