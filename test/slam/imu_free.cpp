#include "../../src/SLAM/_GLIM.h"
#include "../../src/Module/Module.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <glim/mapping/callbacks.hpp>
#include <glim/odometry/callbacks.hpp>

using namespace kai;
namespace fs = std::filesystem;

namespace
{
class TestModules : public ModuleMgr
{
public:
	void add(BASE *module, const string &name) { module->setName(name); m_vModules.push_back(module); }
	~TestModules() { m_vModules.clear(); }
};

class Replies : public _JSONbase
{
public:
	json last;
	bool sendJson(const json &reply) override { last = reply; return true; }
};

class TestGLIM : public _GLIM
{
public:
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

json moduleConfig(const string &name, const string &type)
{
	return {{"name", name}, {"class", type}, {"thread", {{"name", "thread"}, {"class", "_Thread"}, {"FPS", 100}}}};
}

json readJson(const fs::path &path)
{
	std::ifstream in(path);
	assert(in.good());
	json j; in >> j; return j;
}

void writeJson(const fs::path &path, const json &j)
{
	std::ofstream out(path);
	out << j.dump(2);
	assert(out.good());
}

void roomFrame(_PointCloud &cloud, const Isometry3d &pose, uint64_t stamp)
{
	cloud.frameStart();
	// Three intersecting planes constrain all six pose dimensions. These are
	// complete depth exposures: every point in a frame has the same timestamp.
	const Isometry3d inverse = pose.inverse();
	for (int u = -12; u <= 12; ++u)
		for (int v = -12; v <= 12; ++v)
			for (const Vector3d &world : {Vector3d(u * .15, v * .15, 3),
				Vector3d(3, u * .15, v * .15), Vector3d(u * .15, 3, v * .15)})
				cloud.add((inverse * world).cast<float>(), Vector3f::Ones(), stamp);
	cloud.frameStop();
}

void checkNoIMU(const json &status)
{
	assert(status["imuSamples"] == 0 && status["maxIMUgapUs"] == 0);
	assert(status["poseValid"] && status["state"] == "tracking");
}
}

int main(int argc, char **argv)
{
	assert(argc == 3);
	const fs::path source = fs::canonical(argv[1]);
	const fs::path fixture = fs::absolute(argv[2]);
	assert(source != fixture);
	fs::create_directories(fixture);
	// GLIM uses a process-global profile; keep this estimator in its own test
	// executable and copy all configuration before making fixture-only edits.
	for (const auto &entry : fs::directory_iterator(source))
		if (entry.is_regular_file())
			fs::copy_file(entry.path(), fixture / entry.path().filename(), fs::copy_options::overwrite_existing);
	const auto profile = readJson(fixture / "config.json")["global"];
	const fs::path odometryPath = fixture / profile["config_odometry"].get<string>();
	auto odometry = readJson(odometryPath);
	const auto sourceOdometry = odometry;
	assert(odometry["odometry_estimation"]["so_name"] == "libodometry_estimation_ct.so");
	odometry["odometry_estimation"]["smoother_lag"] = .3;
	writeJson(odometryPath, odometry);
	assert(!readJson(fixture / profile["config_sub_mapping"].get<string>())["sub_mapping"]["enable_imu"].get<bool>());
	assert(!readJson(fixture / profile["config_global_mapping"].get<string>())["global_mapping"]["enable_imu"].get<bool>());
	const fs::path preprocessPath = fixture / profile["config_preprocess"].get<string>();
	auto preprocess = readJson(preprocessPath);
	preprocess["preprocess"]["downsample_resolution"] = .1;
	writeJson(preprocessPath, preprocess);
	const fs::path controls = fixture / "test.controls.json";
	fs::remove(controls);

	_PointCloud cloud, map;
	auto cloudConfig = moduleConfig("cloud", "_PointCloud"); cloudConfig["nP"] = 4000;
	assert(cloud.init(cloudConfig) && map.init(moduleConfig("map", "_PointCloud")));
	TestGLIM slam;
	auto config = moduleConfig("slam", "_GLIM");
	config["configPath"] = fixture.string();
	config["fConfig"] = controls.string();
	config["_PointCloud"] = "cloud";
	config["globalMapPCL"] = "map";
	config["nMapPoints"] = 10000;
	config["nLiveFrames"] = 1; // Export preview must not determine stop-time tail retention.
	config["tMapUpdateUs"] = 0;
	config["bPublishLiveMap"] = false;
	config["tConfidenceTimeoutUs"] = 1000000;
	// No _IMUbase module, link or fabricated gravity samples exist in this test.
	assert(slam.init(config));
	TestModules modules;
	modules.add(&cloud, "cloud"); modules.add(&map, "map");
	slam.setName("slam");
	assert(slam.link(config, &modules));
	Replies replies;
	auto command = [&](const string &cmd, const json &values = json()) {
		json request = {{"cmd", cmd}, {"requestId", 42}};
		if (!values.is_null()) request["config"] = values;
		slam.console(request, &replies);
		assert(replies.last["module"] == "slam" && replies.last["requestId"] == 42);
		return replies.last;
	};
	const auto defaults = command("getConfig")["config"];
	assert(defaults.contains("odometry"));
	assert(defaults["odometry"].contains("voxelResolution") && defaults["odometry"].contains("iterations") && defaults["odometry"].contains("threads"));
	assert(!command("setConfig", {{"odometry", {{"iterations", 0}}}})["bSuccess"].get<bool>());
	assert(command("getConfig")["config"] == defaults);
	assert(command("saveConfig", defaults)["bSuccess"] && fs::exists(controls));
	assert(command("setConfig", {{"odometry", {{"iterations", 9}, {"threads", 1}, {"voxelResolution", .2}}}})["bSuccess"]);
	assert(command("getConfig")["config"]["odometry"]["iterations"] == 9);
	assert(command("loadConfig")["bSuccess"] && command("getConfig")["config"] == defaults);

	int inserted = 0;
	vector<long> mappedIds;
	const int inputCallback = glim::OdometryEstimationCallbacks::on_insert_frame.add(
		[&](const glim::PreprocessedFrame::ConstPtr &) { ++inserted; });
	const int mappingCallback = glim::GlobalMappingCallbacks::on_insert_submap.add(
		[&](const glim::SubMap::ConstPtr &submap) {
			for (const auto &frame : submap->odom_frames) mappedIds.push_back(frame->id);
		});
	assert(command("start")["bSuccess"]);
	assert(!command("setConfig", {{"odometry", {{"iterations", 9}}}})["bSuccess"].get<bool>());
	assert(!command("saveConfig", defaults)["bSuccess"].get<bool>());
	assert(!command("loadConfig")["bSuccess"].get<bool>());
	const auto initial = slam.submapSnapshot(0, 0);
	assert(initial.submaps.empty());
	for (int frame = 0; frame < 3; ++frame)
	{
		roomFrame(cloud, Isometry3d::Identity(), 1000000 + frame * 100000);
		slam.step();
		// The first depth frame initializes immediately without waiting for IMU.
		assert(inserted == frame + 1 && slam.status()["frames"] == frame + 1);
		checkNoIMU(slam.status());
		assert(slam.confidence() > 0 && slam.poseConsistent());
		assert(slam.getPos().norm() < .01 && slam.getOrientation().angularDistance(Quaterniond::Identity()) < .01);
		slam.step();
		assert(inserted == frame + 1); // Duplicate ring frame is ignored.
	}
	assert(mappedIds.empty()); // All three frames still lie inside the CT lag.
	size_t exported = 0;
	string error;
	assert(slam.savePointCloud((fixture / "running.ply").string(), exported, error) && exported > 0);
	assert(command("stop")["bSuccess"]);
	assert((mappedIds == vector<long>{0, 1, 2})); // Even nLiveFrames=1 must flush the entire tail once.
	assert(command("stop")["bSuccess"] && mappedIds.size() == 3);
	const auto stopped = slam.submapSnapshot(initial.session, initial.revision);
	assert(stopped.session == initial.session && stopped.revision > initial.revision && !stopped.submaps.empty());
	assert(stopped.submaps.back().timestampUs >= 1199999);
	assert(slam.submapSnapshot(stopped.session, stopped.revision).submaps.empty());
	const auto replay = slam.submapSnapshot(0, 0);
	assert(replay.submaps.front().points == stopped.submaps.front().points);
	assert(slam.savePointCloud((fixture / "completed.ply").string(), exported, error) && exported > 0);
	std::ifstream ply(fixture / "completed.ply"); string header; std::getline(ply, header); assert(header == "ply");
	vector<Vector3f> points; uint64_t stamp = 0;
	assert(map.getLastFrame(&points, nullptr, stamp) > 0);
	assert(command("reset")["bSuccess"]);
	assert(!slam.status()["poseValid"].get<bool>() && slam.status()["mapPoints"] == 0);
	assert(map.getLastFrame(&points, nullptr, stamp) == 0);
	const auto cleared = slam.submapSnapshot(stopped.session, stopped.revision);
	assert(cleared.session != stopped.session && cleared.submaps.empty());
	assert(!slam.savePointCloud((fixture / "empty.ply").string(), exported, error));

	// Even a single depth exposure must survive Stop as a completed submap.
	inserted = 0; mappedIds.clear();
	assert(command("start")["bSuccess"]);
	roomFrame(cloud, Isometry3d::Identity(), 2000000);
	slam.step();
	assert(inserted == 1 && mappedIds.empty());
	assert(command("stop")["bSuccess"]);
	assert((mappedIds == vector<long>{0}));
	const auto single = slam.submapSnapshot(0, 0);
	assert(single.submaps.size() == 1 && single.submaps.front().timestampUs == 2000000);
	assert(command("reset")["bSuccess"]);

	// The same UI parameters must control IMU-free submap creation during motion.
	assert(command("setConfig", {{"submap", {{"keyframes", 2}, {"keyframeStrategy", "DISPLACEMENT"},
		{"keyframeTranslation", .03}, {"keyframeRotation", .03}}}})["bSuccess"]);
	inserted = 0; mappedIds.clear();
	assert(command("start")["bSuccess"]);
	double maxPositionError = 0, maxAngleError = 0;
	for (int frame = 0; frame <= 90; ++frame)
	{
		const double phase = M_PI / 4 * std::max(0.0, frame * .1 - 1.0);
		Isometry3d truth = Isometry3d::Identity();
		truth.translation().x() = .25 * (1 - std::cos(phase));
		truth.linear() = AngleAxisd(.2 * (1 - std::cos(phase)), Vector3d::UnitZ()).toRotationMatrix();
		roomFrame(cloud, truth, 10000000 + frame * 100000);
		slam.step();
		checkNoIMU(slam.status());
		assert(slam.poseConsistent() && slam.confidence() > 0 && inserted == frame + 1);
		maxPositionError = std::max(maxPositionError, (slam.getPos() - truth.translation()).norm());
		maxAngleError = std::max(maxAngleError, Quaterniond(truth.linear()).angularDistance(slam.getOrientation()));
	}
	std::cout << "IMU-free trajectory max error: " << maxPositionError << " m, "
		<< maxAngleError * 180 / M_PI << " degrees\n";
	assert(maxPositionError < .08 && maxAngleError < .04);
	assert(slam.submapSnapshot(0, 0).submaps.size() >= 2);
	assert(mappedIds.size() < 91); // A live tail remains for stop to finalize.
	assert(command("stop")["bSuccess"] && mappedIds.size() == 91);
	for (size_t i = 0; i < mappedIds.size(); ++i) assert(mappedIds[i] == long(i));
	const auto completed = slam.submapSnapshot(0, 0);
	assert(!completed.submaps.empty() && completed.submaps.back().timestampUs >= 18999999);
	for (const auto &submap : completed.submaps)
	{
		assert(submap.points && !submap.points->empty() && submap.pose.matrix().allFinite());
		for (const auto &point : *submap.points) assert(point.allFinite());
	}
	glim::OdometryEstimationCallbacks::on_insert_frame.remove(inputCallback);
	glim::GlobalMappingCallbacks::on_insert_submap.remove(mappingCallback);

	// A new adapter reads persisted controls without changing the source profile.
	TestGLIM restored;
	config["parameters"] = {{"odometry", {{"iterations", 19}}}};
	assert(restored.init(config));
	restored.console({{"cmd", "getConfig"}}, &replies);
	assert(replies.last["config"] == defaults);
	assert(readJson(source / profile["config_odometry"].get<string>()) == sourceOdometry);
	std::cout << "IMU-free initialization, motion, submaps, exact stop flush, export and parameter checks passed\n";
}
