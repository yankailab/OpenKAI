#include "../../src/SLAM/_GLIM.h"
#include "../../src/Module/Module.h"
#include <cassert>
#include <filesystem>
#include <thread>
#include <glim/odometry/callbacks.hpp>

using namespace kai;
namespace fs = std::filesystem;

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
	assert(cloud.getLastFrame(&points, &colors, stamp) == 12);
	assert(stamp == 12345 && points.front().x() == 0 && points.back().x() == 11);
	cloud.frameStart();
	cloud.add(Vector3f(99, 1, 2), color, 23456);
	assert(cloud.getLastFrame(&points, nullptr, stamp) == 12 && stamp == 12345);
	cloud.frameStop();
	assert(cloud.getLastFrame(&points, nullptr, stamp) == 1 && stamp == 23456);
	cloud.clear();
	assert(cloud.getLastFrame(&points, nullptr, stamp) == 0 && stamp == 0);

	// Readers must never observe a partially constructed frame.
	std::atomic_bool done{false};
	std::thread producer([&] {
		for (uint64_t t = 1; t <= 300; ++t)
		{
			cloud.frameStart();
			for (int i = 0; i < 20; ++i) cloud.add(Vector3f(t, i, 1), color, t);
			cloud.frameStop();
		}
		done = true;
	});
	while (!done)
	{
		const int n = cloud.getLastFrame(&points, nullptr, stamp);
		if (!n) continue;
		assert(n == 20);
		for (const auto &p : points) assert(p.x() == stamp);
	}
	producer.join();

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

	_PointCloud cloud;
	_IMUbase imu;
	assert(cloud.init(config("cloud", "_PointCloud")));
	assert(imu.init(config("imu", "_IMUbase")));
	TestGLIM slam;
	auto j = config("slam", "_GLIM");
	j["configPath"] = fixture.string();
	j["tConfidenceTimeoutUs"] = 1000000;
	assert(slam.init(j));
	slam.attach(&cloud, nullptr);
	assert(!slam.startTracking()); // CPU odometry must reject a missing IMU.
	slam.attach(&cloud, &imu);
	assert(slam.startTracking() && slam.startTracking());
	assert(slam.bTracking() && slam.confidence() == 0);
	assert(!slam.saveMap((fixture / "active-map").string()));

	int frames = 0;
	const int callback = glim::OdometryEstimationCallbacks::on_insert_frame.add(
		[&](const glim::PreprocessedFrame::ConstPtr &) { ++frames; });
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
		slam.step();
		assert(frames == frame + 1); // duplicate cloud is not processed twice
	}
	glim::OdometryEstimationCallbacks::on_insert_frame.remove(callback);
	assert(slam.getPos().norm() < 0.1);
	std::this_thread::sleep_for(std::chrono::milliseconds(1100));
	assert(slam.confidence() == 0);
	const Vector3d pose = slam.getPos();
	slam.stopTracking();
	assert(!slam.bTracking() && slam.confidence() == 0 && slam.getPos().isApprox(pose));
	assert(slam.saveMap((fixture / "map").string()));
	assert(fs::exists(fixture / "map" / "graph.bin"));
	slam.reset();
	assert(slam.getPos().isZero() && slam.getOrientation().isApprox(Quaterniond::Identity()));
	assert(!slam.saveMap((fixture / "reset-map").string()));
	assert(slam.startTracking());
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
	std::cout << "SLAM input, lifecycle, CPU pose and mapping checks passed\n";
}
