#include "../../src/SLAM/_GLIM.h"
#include "../../src/Module/Module.h"
#include <cassert>
#include <filesystem>
#include <thread>
#include <glim/odometry/callbacks.hpp>

using namespace kai;
namespace fs = std::filesystem;

class TestModules : public ModuleMgr
{
public:
	void add(BASE *module, const string &name) { module->setName(name); m_vModules.push_back(module); }
	~TestModules() { m_vModules.clear(); } // Test modules have stack ownership.
};

class Replies : public _JSONbase
{
public:
	json last;
	bool sendJson(const json &reply) override { last = reply; return true; }
};

class BusySLAM : public _SLAMbase
{
public:
	std::atomic_int steps{0};
	bool check() override { return true; }
	bool startSLAM() override { return true; }
	void updateSLAM() override
	{
		++steps;
		std::this_thread::sleep_for(std::chrono::milliseconds(3));
	}
};

json config(const string &name, const string &type)
{
	return {{"name", name}, {"class", type}, {"thread", {{"name", "thread"}, {"class", "_Thread"}, {"FPS", 100}}}};
}

class TestGLIM : public _GLIM
{
public:
	void attach(_PointCloud *cloud, _IMUbase *imu)
	{
		m_pPCL = cloud;
		m_pIMU = imu;
	}
	void step()
	{
		std::lock_guard<std::mutex> lock(m_mtxSLAM);
		if (bTracking()) updateSLAM();
	}
	bool poseConsistent() const
	{
		return m_mPose.translation().isApprox(m_vPos) &&
			m_mPose.linear().isApprox(m_vOrt.toRotationMatrix()) &&
			m_mPosef.isApprox(m_mPose.cast<float>());
	}
};

void writeJson(const fs::path &path, const json &j)
{
	std::ofstream out(path);
	out << j.dump(2);
	assert(out.good());
}

void sensorTests()
{
	_PointCloud cloud;
	auto j = config("cloud", "_PointCloud");
	j["nP"] = 3;
	assert(cloud.init(j));
	const Vector3f color(1, 1, 1);
	cloud.frameStart();
	for (int i = 0; i < 12; ++i) cloud.add(Vector3f(i, 1, 2), color, 12345);
	cloud.frameStop();
	vector<Vector3f> points, colors;
	uint64_t stamp = 0;
	assert(cloud.getLastFrame(&points, &colors, stamp) == 3);
	assert(stamp == 12345 && points.front().x() == 9 && points.back().x() == 11);
	assert(colors.size() == 3 && colors.front().isApprox(color));
	cloud.frameStart();
	cloud.add(Vector3f(99, 1, 2), color, 23456);
	assert(cloud.getLastFrame(&points, nullptr, stamp) == 0 && stamp == 0);
	cloud.frameStop();
	assert(cloud.getLastFrame(&points, nullptr, stamp) == 1 && stamp == 23456);
	cloud.frameStart();
	cloud.add(Vector3f(7, 1, 2), color, 34567);
	cloud.add(Vector3f(8, 1, 2), color, 34567);
	// The preceding frame remains readable until its own slots are overwritten.
	assert(cloud.getLastFrame(&points, nullptr, stamp) == 1 && points.front().x() == 99);
	cloud.frameStop();
	assert(cloud.getLastFrame(&points, nullptr, stamp) == 2 && stamp == 34567);
	assert(points.front().x() == 7 && points.back().x() == 8); // wrapped end index
	cloud.frameStart();
	cloud.frameStop();
	assert(cloud.getLastFrame(&points, &colors, stamp) == 0 && colors.empty() && stamp == 0);
	cloud.clear();
	assert(cloud.getLastFrame(&points, nullptr, stamp) == 0 && stamp == 0);

	// A reader sees a complete ring span or no frame after overwrite, never a mix.
	std::atomic_bool done{false};
	std::thread producer([&] {
		for (uint64_t t = 1; t <= 300; ++t)
		{
			cloud.frameStart();
			for (int i = 0; i < 2; ++i) cloud.add(Vector3f(t, i, 1), color, t);
			cloud.frameStop();
		}
		done = true;
	});
	while (!done)
	{
		const int n = cloud.getLastFrame(&points, nullptr, stamp);
		if (!n) continue;
		assert(n == 2);
		for (const auto &p : points) assert(p.x() == stamp);
	}
	producer.join();
	cloud.setFrame({Vector3f(1, 2, 3), Vector3f(4, 5, 6)}, {}, 98765);
	assert(cloud.getLastFrame(&points, &colors, stamp) == 2 && stamp == 98765);
	assert(points.back().isApprox(Vector3f(4, 5, 6)) && colors.front().isOnes());
	cloud.setFrame({Vector3f(0, 0, 0), Vector3f(1, 0, 0), Vector3f(2, 0, 0), Vector3f(3, 0, 0)}, {}, 98766);
	assert(cloud.getLastFrame(&points, nullptr, stamp) == 3 && stamp == 98766);
	assert(points.front().x() == 1 && points.back().x() == 3); // full-ring span
	cloud.setFrame({}, {}, 98766);
	assert(cloud.getLastFrame(&points, &colors, stamp) == 0 && colors.empty() && stamp == 98766);

	_IMUbase imu;
	assert(imu.init(config("imu", "_IMUbase")));
	imu.addGyro(Vector3f::Zero(), 100);
	imu.addGyro(Vector3f(1, 2, 3), 10000);
	imu.addAcc(Vector3f(4, 5, 6), 10100);
	Vector3f g, a;
	assert(imu.getIMUpair(&g, &a) == 10100);
	assert(g.isApprox(Vector3f(1, 2, 3)) && a.isApprox(Vector3f(4, 5, 6)));
	assert(imu.getIMUpair(&g, &a) == 0);

	_NavBase nav;
	j = config("nav", "_NavBase");
	j["tConfidenceTimeoutUs"] = 1000;
	assert(nav.init(j));
	nav.setConfidence(200);
	assert(nav.confidence() == 100);
	nav.setConfidence(-2);
	assert(nav.confidence() == 0);
	nav.setConfidence(std::numeric_limits<float>::quiet_NaN());
	assert(nav.confidence() == 0);
	nav.setConfidence(80);
	std::this_thread::sleep_for(std::chrono::milliseconds(5));
	assert(nav.confidence() == 0);
}

int main(int argc, char **argv)
{
	assert(argc == 3);
	sensorTests();
	Module factory;
	std::unique_ptr<BASE> module(factory.createInstance("_GLIM"));
	assert(dynamic_cast<_GLIM *>(module.get()));
	const fs::path source(argv[1]), fixture(argv[2]);
	fs::create_directories(fixture);
	for (const auto &entry : fs::directory_iterator(source))
		fs::copy_file(entry.path(), fixture / entry.path().filename(), fs::copy_options::overwrite_existing);
	json odom;
	std::ifstream(fixture / "config_odometry_cpu.json") >> odom;
	odom["odometry_estimation"]["init_T_world_imu"] = {0, 0, 0, 0, 0, 0, 1};
	odom["odometry_estimation"]["smoother_lag"] = 0.3;
	writeJson(fixture / "config_odometry_cpu.json", odom);
	json preprocess;
	std::ifstream(fixture / "config_preprocess.json") >> preprocess;
	preprocess["preprocess"]["downsample_resolution"] = 0.1;
	writeJson(fixture / "config_preprocess.json", preprocess);
	// Synthetic fixtures use a known IMU/cloud transform, independently of
	// the per-device factory calibration in the live camera profile.
	json sensors;
	std::ifstream(fixture / "config_sensors.json") >> sensors;
	sensors["sensors"]["T_lidar_imu"] = {0, 0, 0, 0, 0, 0, 1};
	writeJson(fixture / "config_sensors.json", sensors);

	_PointCloud cloud;
	_PointCloud map;
	_IMUbase imu;
	auto cloudConfig = config("cloud", "_PointCloud");
	cloudConfig["nP"] = 4000; // The synthetic frame has 1875 points.
	assert(cloud.init(cloudConfig));
	assert(map.init(config("map", "_PointCloud")));
	assert(imu.init(config("imu", "_IMUbase")));
	TestGLIM slam;
	auto j = config("slam", "_GLIM");
	j["configPath"] = fixture.string();
	j["tConfidenceTimeoutUs"] = 1000000;
	j["_PointCloud"] = "cloud";
	j["_IMUbase"] = "imu";
	j["globalMapPCL"] = "map";
	j["nMapPoints"] = 800;
	j["tMapUpdateUs"] = 0;
	assert(slam.init(j));
	TestModules modules;
	modules.add(&cloud, "cloud"); modules.add(&map, "map"); modules.add(&imu, "imu");
	slam.setName("slam");
	json invalidLink = j; invalidLink["globalMapPCL"] = "cloud";
	assert(!slam.link(invalidLink, &modules));
	assert(slam.link(j, &modules));
	Replies replies;
	auto command = [&](const string &cmd) {
		slam.console({{"cmd", cmd}, {"requestId", 42}}, &replies);
		assert(replies.last["module"] == "slam" && replies.last["requestId"] == 42);
		return replies.last;
	};
	assert(command("getStatus")["status"]["state"] == "stopped");
	assert(!command("unsupported")["bSuccess"].get<bool>());
	slam.attach(&cloud, nullptr);
	assert(!slam.startTracking()); // CPU odometry must reject a missing IMU.
	assert(slam.status()["state"] == "error");
	slam.attach(&cloud, &imu);
	assert(slam.startTracking() && slam.startTracking());
	assert(slam.bTracking() && slam.confidence() == 0);
	assert(command("start")["bSuccess"] && slam.status()["state"] == "initializing");
	assert(!slam.saveMap((fixture / "active-map").string()));

	int frames = 0;
	const int callback = glim::OdometryEstimationCallbacks::on_insert_frame.add(
		[&](const glim::PreprocessedFrame::ConstPtr &) { ++frames; });
	Vector3f correctedFirstPoint = Vector3f::Zero();
	bool correctedPreview = false;
	const int previewCallback = glim::OdometryEstimationCallbacks::on_update_frames.add(
		[&](const vector<glim::EstimationFrame::ConstPtr> &active) {
			if (active.size() != 2 || active.back()->id != 1) return;
			// Simulate a deterministic smoother revision to an older live frame.
			// The source objects are mutable backend frames exposed through const views.
			auto first = std::const_pointer_cast<glim::EstimationFrame>(active.front());
			first->T_world_imu.translation().x() += .02;
			first->T_world_lidar = first->T_world_imu * first->T_lidar_imu.inverse();
			correctedFirstPoint = (first->T_world_sensor() * first->frame->points[0]).head<3>().cast<float>();
			correctedPreview = true;
		});
	uint64_t imuStamp = 990000;
	for (int frame = 0; frame < 12; ++frame)
	{
		const uint64_t stamp = 1000000 + frame * 100000;
		cloud.frameStart();
		// Three intersecting planes constrain all six pose dimensions.
		for (int u = -12; u <= 12; ++u)
			for (int v = -12; v <= 12; ++v)
			{
				cloud.add(Vector3f(u * .15f, v * .15f, 3), Vector3f::Ones(), stamp);
				cloud.add(Vector3f(3, u * .15f, v * .15f), Vector3f::Ones(), stamp);
				cloud.add(Vector3f(u * .15f, 3, v * .15f), Vector3f::Ones(), stamp);
			}
		cloud.frameStop();
		if (frame == 0)
		{
			slam.step();
			assert(frames == 0); // hold the frame until IMU covers it
		}
		for (; imuStamp <= stamp + 10000; imuStamp += 5000)
		{
			imu.addGyro(Vector3f::Zero(), imuStamp);
			imu.addAcc(Vector3f(0, 0, 9.80665), imuStamp);
		}
		slam.step();
		assert(frames == frame + 1);
		assert(slam.confidence() > 0 && slam.poseConsistent());
		const auto status = command("getStatus")["status"];
		assert(status["state"] == "tracking" && status["poseValid"] && status["frames"] == frame + 1);
		vector<Vector3f> mapPoints;
		uint64_t mapStamp = 0;
		const int count = map.getLastFrame(&mapPoints, nullptr, mapStamp);
		assert(count > 0 && count <= 800 && status["mapPoints"] == count && mapStamp > 0);
		for (const auto &point : mapPoints) assert(point.allFinite());
		if (frame == 1) assert(correctedPreview && mapPoints.front().isApprox(correctedFirstPoint, 1e-6));
		slam.step();
		assert(frames == frame + 1); // duplicate cloud is not processed twice
	}
	glim::OdometryEstimationCallbacks::on_insert_frame.remove(callback);
	glim::OdometryEstimationCallbacks::on_update_frames.remove(previewCallback);
	assert(slam.getPos().norm() < 0.1);
	std::this_thread::sleep_for(std::chrono::milliseconds(1100));
	assert(slam.confidence() == 0);
	const Vector3d pose = slam.getPos();
	assert(command("stop")["bSuccess"]);
	assert(!slam.bTracking() && slam.confidence() == 0 && slam.getPos().isApprox(pose));
	assert(slam.status()["mapPoints"].get<int>() > 0 && slam.status()["submaps"].get<int>() > 0);
	assert(slam.saveMap((fixture / "map").string()));
	assert(fs::exists(fixture / "map" / "graph.bin"));
	assert(command("reset")["bSuccess"]);
	vector<Vector3f> mapPoints;
	uint64_t mapStamp = 0;
	assert(map.getLastFrame(&mapPoints, nullptr, mapStamp) == 0);
	assert(slam.status()["mapPoints"] == 0 && !slam.status()["poseValid"].get<bool>());
	assert(slam.getPos().isZero() && slam.getOrientation().isApprox(Quaterniond::Identity()));
	assert(!slam.saveMap((fixture / "reset-map").string()));

	// Translate 0.5 m and rotate 23 degrees, then return to the starting pose.
	// Feed independently synthesized 200 Hz IMU measurements and 10 Hz clouds.
	// This exercises motion through many smoother marginalizations/submaps.
	assert(slam.startTracking());
	double maxPositionError = 0, maxAngleError = 0;
	imuStamp = 9990000;
	vector<glim::EstimationFrame::ConstPtr> activeFrames;
	const int updateCallback = glim::OdometryEstimationCallbacks::on_update_frames.add(
		[&](const vector<glim::EstimationFrame::ConstPtr> &active) { activeFrames = active; });
	const double omega = M_PI / 4;
	auto trajectory = [&](double t) {
		const double phase = omega * std::max(0.0, t - 1.0);
		Isometry3d pose = Isometry3d::Identity();
		pose.translation().x() = .25 * (1 - std::cos(phase));
		pose.linear() = AngleAxisd(.2 * (1 - std::cos(phase)), Vector3d::UnitZ()).toRotationMatrix();
		return pose;
	};
	for (int frame = 0; frame <= 90; ++frame)
	{
		const uint64_t stamp = 10000000 + frame * 100000;
		const Isometry3d truth = trajectory(frame * .1);
		cloud.frameStart();
		for (int u = -12; u <= 12; ++u)
			for (int v = -12; v <= 12; ++v)
				for (const Vector3d &world : {Vector3d(u * .15, v * .15, 3),
					Vector3d(3, u * .15, v * .15), Vector3d(u * .15, 3, v * .15)})
					cloud.add((truth.inverse() * world).cast<float>(), Vector3f::Ones(), stamp);
		cloud.frameStop();
		for (; imuStamp <= stamp + 10000; imuStamp += 5000)
		{
			const double t = (double(imuStamp) - 10000000) * 1e-6;
			Vector3d acceleration(0, 0, 9.80665), angular = Vector3d::Zero();
			if (t >= 1)
			{
				acceleration.x() = .25 * omega * omega * std::cos(omega * (t - 1));
				angular.z() = .2 * omega * std::sin(omega * (t - 1));
			}
			imu.addAcc((trajectory(t).linear().transpose() * acceleration).cast<float>(), imuStamp);
			imu.addGyro(angular.cast<float>(), imuStamp);
		}
		slam.step();
		assert(slam.confidence() > 0);
		maxPositionError = std::max(maxPositionError, (slam.getPos() - truth.translation()).norm());
		maxAngleError = std::max(maxAngleError, Quaterniond(truth.linear()).angularDistance(slam.getOrientation()));
		// Until the first marginalization, the first map point must use the
		// first frame's CURRENT smoothed pose, including later corrections.
		if (frame == 1)
		{
			assert(activeFrames.size() == 2);
			assert(map.getLastFrame(&mapPoints, nullptr, mapStamp) > 0);
			const auto &first = activeFrames.front();
			const Vector3f expected = (first->T_world_sensor() * first->frame->points[0]).head<3>().cast<float>();
			assert(mapPoints.front().isApprox(expected, 1e-6));
		}
	}
	glim::OdometryEstimationCallbacks::on_update_frames.remove(updateCallback);
	activeFrames.clear();
	std::cout << "Moving trajectory max error: " << maxPositionError << " m, "
		<< maxAngleError * 180 / M_PI << " degrees\n";
	assert(maxPositionError < .08 && maxAngleError < .04);
	assert(slam.status()["imuSamples"].get<int>() > 1800 && slam.status()["maxIMUgapUs"] == 5000);
	slam.stopTracking();

	// Exercise the real OpenKAI worker and cooperative shutdown too.
	j["bAutoStart"] = false;
	TestGLIM worker;
	assert(worker.init(j));
	worker.attach(&cloud, &imu);
	assert(worker.start());
	assert(!worker.bTracking());
	assert(worker.startTracking());
	worker.pause();
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	worker.stop();
	assert(!worker.bTracking() && worker.bStopped());

	// An overloaded worker must yield the session lock to pending controls.
	BusySLAM busy;
	j = config("busy", "_SLAMbase"); j["thread"]["FPS"] = 1000;
	assert(busy.init(j) && busy.start());
	while (!busy.steps) std::this_thread::sleep_for(std::chrono::milliseconds(1));
	const auto beforeStop = std::chrono::steady_clock::now();
	busy.stopTracking();
	assert(std::chrono::steady_clock::now() - beforeStop < std::chrono::seconds(1));
	busy.stop();
	std::cout << "SLAM input, lifecycle, CPU pose and mapping checks passed\n";
}
