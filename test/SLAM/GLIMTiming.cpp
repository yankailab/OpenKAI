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
		void allowPointRead() { m_tPointRead = 0; }
		void inputs(PCLframe &points, IMUstream &imu)
		{
			m_pPCLin = &points;
			m_pIMUin = &imu;
		}
	};

	class PointHarness : public _SLAMbase
	{
	public:
		using _SLAMbase::readPointCloud;
		void input(PCLframe &points) { m_pPCLin = &points; }
		void allowPointRead() { m_tPointRead = 0; }
		void scanDuration(uint64_t duration) { m_scanDurationMs = duration; }
	};

	void pointWindowTest()
	{
		PCLframe stream;
		assert(stream.clear(8));
		PointHarness reader;
		reader.input(stream);
		reader.scanDuration(100);
		auto point = [](uint64_t stamp) {
			GEOMETRY_POINT value;
			value.m_vP = Vector3f(1, 2, 3);
			value.m_tStamp = stamp;
			return value;
		};
		const uint64_t begin = 10 * NSEC_SEC;
		vector<GEOMETRY_POINT> points;
		uint64_t stamp = 0;
		stream.add({point(begin), point(begin + 10 * NSEC_MSEC),
			point(0), point(begin + 50 * NSEC_MSEC)}, 1000);
		assert(reader.readPointCloud(points, stamp));
		assert(points.size() == 3 && stamp == begin);
		assert(!reader.readPointCloud(points, stamp));

		// The ring still retains the first batch, but each capture is consumed once.
		stream.add({point(begin + 60 * NSEC_MSEC), point(begin + 110 * NSEC_MSEC),
			point(begin + 120 * NSEC_MSEC)}, 2000);
		assert(!reader.readPointCloud(points, stamp)); // consumer scan cadence
		reader.allowPointRead();
		assert(reader.readPointCloud(points, stamp));
		assert(points.size() == 3 && stamp == begin + 60 * NSEC_MSEC);

		// A slow consumer takes the latest retained scan window, in timestamp order.
		vector<GEOMETRY_POINT> batch;
		for (uint64_t offset = 200; offset < 400; offset += 10)
			batch.push_back(point(begin + offset * NSEC_MSEC));
		stream.add(batch, 3000);
		reader.allowPointRead();
		assert(reader.readPointCloud(points, stamp));
		assert(points.size() == 8 && stamp == begin + 320 * NSEC_MSEC);
		assert(stream.clear(20));
		reader.reset();
		stream.add(batch, 3500);
		assert(reader.readPointCloud(points, stamp));
		assert(points.size() == 11 && stamp == begin + 290 * NSEC_MSEC);
		stream.updateTstamp(4000);
		reader.allowPointRead();
		assert(!reader.readPointCloud(points, stamp));

		// A reset is visible even while the ring retains larger old-epoch stamps.
		stream.add({point(NSEC_SEC), point(NSEC_SEC + 10 * NSEC_MSEC)}, 5000);
		reader.allowPointRead();
		bool rejected = false;
		try { reader.readPointCloud(points, stamp); }
		catch (const std::runtime_error &) { rejected = true; }
		assert(rejected);
		reader.reset();
		assert(reader.readPointCloud(points, stamp));
		assert(points.size() == 2 && stamp == NSEC_SEC);

		assert(stream.clear());
		reader.allowPointRead();
		assert(!reader.readPointCloud(points, stamp));
	}

	void cameraFrameTest()
	{
		PCLframe stream;
		PointHarness reader;
		reader.input(stream);
		vector<GEOMETRY_POINT> frame(2);
		frame[0].m_vP = frame[1].m_vP = Vector3f(1, 2, 3);
		vector<GEOMETRY_POINT> output;
		uint64_t stamp = 0;
		for (uint64_t capture = NSEC_SEC; capture < NSEC_SEC + 100 * NSEC_MSEC; capture += 33 * NSEC_MSEC)
		{
			frame[0].m_tStamp = frame[1].m_tStamp = capture;
			stream.set(frame, capture + 10 * NSEC_SEC);
			assert(reader.readPointCloud(output, stamp));
			assert(output.size() == 2 && stamp == capture);
		}
	}

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
	pointWindowTest();
	cameraFrameTest();
	const bool globalShutter = string(argv[2]) == "global";
	JsonCfg config;
	config.setJson({{"GLIM", {{"class", "_GLIM"}, {"configPath", argv[1]},
		{"thread", {{"FPS", 0}}}, {"bAutoStart", false}, {"bMapping", false}, {"scanDurationMs", 100},
		{"parameters", {{"preprocess", {{"distanceNear", .5}, {"distanceFar", 100},
			{"voxelResolution", .02}, {"targetPoints", 1000}, {"threads", 1}}}}}}}});
	PCLframe points;
	assert(points.clear(1024));
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
	points.add(cloud(begin), begin);
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

	// Publication timestamps belong to the host clock; capture time still anchors
	// rolling-scan offsets even when it precedes the publication by a large amount.
	points.add(cloud(begin + NSEC_SEC), begin + 30 * NSEC_SEC);
	for (uint64_t t = end + 1; t <= begin + NSEC_SEC + 110 * NSEC_MSEC; t += 5 * NSEC_MSEC)
		sample(imu, t);
	// The old cloud remains in the ring during packet streaming. None of those
	// captures may be inserted again into the estimator.
	slam.allowPointRead();
	slam.updateSLAM();
	assert(arrivals == 2 && raw && raw->size() == 400);
	assert(std::abs(raw->stamp - nsec2sec<double>(begin + NSEC_SEC)) < 1e-6);
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
