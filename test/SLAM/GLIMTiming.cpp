#include "../../src/SLAM/_GLIM.h"
#include <glim/preprocess/callbacks.hpp>

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

namespace
{
	using namespace kai;

	class GLIMHarness : public _GLIM
	{
	public:
		using _GLIM::updateSLAM;
		using _SLAMbase::readIMU;
		void inputs(PCLframe &points, IMUstream &imu)
		{
			m_pPCLin = &points;
			m_pIMUin = &imu;
		}
	};

	void sample(IMUstream &imu, uint64_t stamp)
	{
		imu.addAcc(Vector3f(0, 0, 9.80665f), stamp);
		imu.addGyro(Vector3f::Zero(), stamp);
	}

	vector<GEOMETRY_POINT> cloud(uint64_t stamp)
	{
		vector<GEOMETRY_POINT> points;
		for (int i = 0; i < 400; ++i)
		{
			GEOMETRY_POINT point;
			point.m_vP = Vector3f(2.0f + (i % 20) * .1f, (i / 20) * .1f, .2f);
			point.m_tStamp = stamp + uint64_t(i) * 250000;
			points.push_back(point);
		}
		// Filtering must preserve each retained coordinate/time pair and must
		// not make IMU coverage depend on invalid samples outside the scan.
		GEOMETRY_POINT invalid = points.front();
		invalid.m_vP.x() = std::numeric_limits<float>::quiet_NaN();
		invalid.m_tStamp = stamp + NSEC_SEC;
		points.insert(points.begin() + 5, invalid);
		invalid.m_vP = Vector3f(200, 0, 0);
		points.push_back(invalid);
		invalid.m_vP.setZero();
		points.push_back(invalid);
		invalid.m_vP = Vector3f(1, 0, 0);
		invalid.m_tStamp = 0;
		points.push_back(invalid);
		return points;
	}
}

int main(int argc, char **argv)
{
	assert(argc == 3);
	const bool globalShutter = string(argv[2]) == "global";
	JsonCfg config;
	config.setJson({{"GLIM", {{"class", "_GLIM"}, {"configPath", argv[1]},
		{"thread", {{"FPS", 0}}}, {"bAutoStart", false}, {"bMapping", false},
		{"parameters", {{"preprocess", {{"distanceNear", .5}, {"distanceFar", 100},
			{"voxelResolution", .02}, {"targetPoints", 1000}, {"threads", 1}}}}}}}});
	PCLframe points;
	IMUstream imu;
	GLIMHarness slam;
	slam.setName("GLIM");
	slam.setConfig(&config, &(*config.getJson())["GLIM"]);
	assert(slam.loadConfig());
	slam.inputs(points, imu);
	assert(slam.startTracking());

	glim::RawPoints::ConstPtr raw;
	int arrivals = 0;
	const int rawCallback = glim::PreprocessCallbacks::on_raw_points_received.add(
		[&](const glim::RawPoints::ConstPtr &value) { raw = value; ++arrivals; });
	double filteredEnd = -1;
	const int filteredCallback = glim::PreprocessCallbacks::on_filtering_finished.add(
		[&](gtsam_points::PointCloudCPU::Ptr &value) {
			assert(value->size() >= 100);
			filteredEnd = value->times[value->size() - 1];
		});

	// An epoch-sized stamp catches precision loss from subtracting floating
	// point seconds instead of the original integer nanoseconds.
	const uint64_t begin = 1800000000123456789ULL;
	const uint64_t end = begin + 399 * 250000;
	points.set(cloud(begin), begin);
	for (uint64_t t = begin - 20000000; t <= begin + 50000000; t += 5000000)
		sample(imu, t);
	slam.updateSLAM();
	assert(arrivals == (globalShutter ? 1 : 0));
	if (!globalShutter)
	{
		// An IMU sample exactly on the last point still needs an upper bracket.
		sample(imu, end);
		slam.updateSLAM();
		assert(arrivals == 0);
		sample(imu, end + 1);
		slam.updateSLAM();
		assert(arrivals == 1);
	}
	assert(raw && raw->size() == 400);
	assert(raw->times.size() == raw->points.size());
	for (size_t i = 0; i < raw->times.size(); ++i)
	{
		const double expected = globalShutter ? 0.0 : double(i) * .00025;
		assert(std::abs(raw->times[i] - expected) < 1e-12);
		assert(std::abs(raw->points[i].x() - (2.0f + (i % 20) * .1f)) < 1e-6);
	}
	assert(globalShutter ? filteredEnd == 0 : filteredEnd > .09);
	slam.updateSLAM();
	assert(arrivals == 1); // Polling cannot feed the same frame twice.

	if (!globalShutter)
	{
		auto bad = cloud(begin + NSEC_SEC);
		bad.front().m_tStamp = begin + NSEC_SEC - 1;
		points.set(bad, begin + NSEC_SEC);
		bool rejected = false;
		try { slam.updateSLAM(); }
		catch (const std::runtime_error &) { rejected = true; }
		assert(rejected); // Must not underflow an unsigned timestamp offset.
	}
	glim::PreprocessCallbacks::on_raw_points_received.remove(rawCallback);
	glim::PreprocessCallbacks::on_filtering_finished.remove(filteredCallback);
	slam.stopTracking();

	// A reboot restarts the sensor clock. Flushing both producer histories
	// lets a newly started SLAM session consume the new epoch immediately.
	imu.clear();
	deque<IMUstream::IMU_DATA> remainingGyro, remainingAcc;
	imu.get(remainingGyro, remainingAcc);
	assert(remainingGyro.empty() && remainingAcc.empty());
	const uint64_t rebootStamp = 10 * NSEC_SEC;
	sample(imu, rebootStamp);
	sample(imu, rebootStamp + 5000000);
	assert(slam.startTracking());
	Vector3d acc, gyro;
	uint64_t stamp = 0;
	assert(slam.readIMU(acc, gyro, stamp) && stamp == rebootStamp);
	assert(slam.readIMU(acc, gyro, stamp) && stamp == rebootStamp + 5000000);
	assert(!slam.readIMU(acc, gyro, stamp));
	slam.stopTracking();
	std::cout << "GLIM " << (globalShutter ? "global-shutter" : "rolling-scan")
		<< " timing, IMU coverage, and clock restart passed\n";
}
