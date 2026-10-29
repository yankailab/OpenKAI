#include "../../src/Universe/_TransformRigid.h"
#include "../../src/Universe/Geometry/PointCloud/Pipeline/_PCmerge.h"
#include "../../src/Universe/Geometry/PointCloud/_PCLfile.h"
#ifdef WITH_PROTOCOL
#include "../../src/Protocol/_JSONbase.h"
#endif
#ifdef WITH_SENSOR
#include "../../src/Sensor/LiDAR/Livox/_Livox2.h"
#include "../../src/Sensor/LiDAR/RoboSense/Airy/_Airy.h"
#include "../../src/Sensor/LiDAR/RoboSense/E1R/_E1R.h"
#endif
#ifdef WITH_SLAM
#include "../../src/SLAM/_SLAMbase.h"
#endif
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <thread>
#include <unistd.h>

#if defined(WITH_VISION) && defined(USE_OPENCV)
void testRGBDTransform(void);
#endif

namespace
{
	using namespace kai;

	class TransformHarness : public _TransformRigid
	{
	public:
		bool parent(TransformHarness &parent)
		{
			m_pTfParent = &parent;
			parent.m_vpTf.push_back(this);
			return true;
		}

		void catchUp(void) override
		{
			++m_wakeups;
			_ModuleBase::catchUp();
		}

		Isometry3d cachedPose(void)
		{
			std::lock_guard<std::recursive_mutex> lock(m_mtxPose);
			return m_mPoseCombined;
		}

		std::atomic<unsigned> m_wakeups{0};
	};

	class MergeHarness : public _PCmerge
	{
	public:
		~MergeHarness() { join(); }

		void join(void)
		{
			if (m_pT)
			{
				m_pT->join();
			}
		}
	};

	template <typename Consumer>
	class SaveHarness : public Consumer
	{
	public:
		void transform(_TransformRigid *value) { this->m_pTf = value; }
	};

	json readConfig(const string &path)
	{
		JsonCfg cfg;
		assert(cfg.readFromFile(path));
		return *cfg.getJson();
	}

	void assertSavedPose(const json &j, const Vector3d &translation, const Quaterniond &rotation)
	{
		assert(j.at("vT") == json({translation.x(), translation.y(), translation.z()}));
		const auto &q = j.at("vR");
		assert(q.size() == 4);
		const Quaterniond saved(q[3].get<double>(), q[0].get<double>(), q[1].get<double>(), q[2].get<double>());
		assert(std::abs(saved.norm() - 1.0) < 1e-12);
		assert(saved.coeffs().isApprox(rotation.coeffs(), 1e-12));
	}

	template <typename Consumer>
	void testConsumerConfigIsolation(const string &className, json ownedFields,
		const json &reloadFields = json::object())
	{
		char directory[] = "/tmp/openkai-transform-save-XXXXXX";
		assert(mkdtemp(directory));
		const string consumerPath = string(directory) + "/consumer.json";
		const string transformPath = string(directory) + "/transform.json";
		JsonCfg consumerCfg, transformCfg;
		json settings = ownedFields;
		settings["class"] = className;
		settings["_TransformRigid"] = "mount";
		consumerCfg.setJson({{"consumer", settings}, {"unrelated", "preserve consumer document"}});
		transformCfg.setJson({{"mount", {{"class", "_TransformRigid"}, {"vT", {0, 0, 0}},
			{"vR", {0, 0, 0.5, 0.5}}}}, {"unrelated", "preserve transform document"}});
		assert(consumerCfg.saveToFile(consumerPath));
		assert(transformCfg.saveToFile(transformPath));
		// readFromFile binds the filename used by saveConfig(true).
		assert(consumerCfg.readFromFile(consumerPath));
		assert(transformCfg.readFromFile(transformPath));
		const json consumerOnDisk = readConfig(consumerPath);
		const json transformOnDisk = readConfig(transformPath);
		json &consumerJson = (*consumerCfg.getJson())["consumer"];
		json &transformJson = (*transformCfg.getJson())["mount"];
		TransformHarness parent, transform;
		parent.setTranslation(100, 0, 0);
		parent.setRotation(0.5, 0, 0, 0.5);
		transform.setName("mount");
		transform.setConfig(&transformCfg, &transformJson);
		assert(transform.loadConfig());
		transform.parent(parent);
		transform.setTranslation(1, 2, 3);
		transform.setRotation(0, 0, 0.5, 0.5);
		const Quaterniond rotation = Quaterniond(0.5, 0, 0, 0.5).normalized();
		const json transformInMemory = *transformCfg.getJson();
		const Isometry3d originalPose = transform.getPose();
		const Isometry3d originalCombined = transform.getPoseCombined();
		assert(!originalCombined.matrix().isApprox(originalPose.matrix()));
		const auto assertTransformUnchanged = [&](const Isometry3d &pose, const Isometry3d &combined)
		{
			assert(transform.getPose().matrix().isApprox(pose.matrix()));
			assert(transform.getPoseCombined().matrix().isApprox(combined.matrix()));
			assert(*transformCfg.getJson() == transformInMemory);
			assert(readConfig(transformPath) == transformOnDisk);
		};
		SaveHarness<Consumer> consumer;
		consumer.setName("consumer");
		consumer.BASE::setConfig(&consumerCfg, &consumerJson);
		consumer.transform(&transform);
		assert(consumer.loadConfig());
		assertTransformUnchanged(originalPose, originalCombined);
		if (!reloadFields.empty())
		{
			consumerJson.update(reloadFields);
			ownedFields.update(reloadFields);
			assert(consumer.loadConfig());
			assertTransformUnchanged(originalPose, originalCombined);
		}
		// Only the consumer's own settings are restored in memory or exported.
		for (const auto &field : ownedFields.items()) consumerJson.erase(field.key());
		assert(consumer.saveConfig(false));
		assertTransformUnchanged(originalPose, originalCombined);
		for (const auto &field : ownedFields.items()) assert(consumerJson.at(field.key()) == field.value());
		assert(!consumerJson.contains("vT") && !consumerJson.contains("vR"));
		assert(readConfig(consumerPath) == consumerOnDisk);

		// An unsaved pose change must remain local to its owner when a consumer exports.
		transform.setTranslation(4, 5, 6);
		const Isometry3d changedPose = transform.getPose();
		const Isometry3d changedCombined = transform.getPoseCombined();
		assert(consumer.saveConfig(true));
		assertTransformUnchanged(changedPose, changedCombined);
		assert(readConfig(consumerPath) == *consumerCfg.getJson());
		assert((*consumerCfg.getJson())["unrelated"] == consumerOnDisk.at("unrelated"));
		assert(!consumerJson.contains("vT") && !consumerJson.contains("vR"));

		// The transform can still save its own normalized local pose independently.
		const json savedConsumer = *consumerCfg.getJson();
		assert(transform.saveConfig(false));
		assertSavedPose(transformJson, Vector3d(4, 5, 6), rotation);
		assert(readConfig(transformPath) == transformOnDisk);
		assert(transform.saveConfig(true));
		const json savedTransform = *transformCfg.getJson();
		assert(readConfig(transformPath) == savedTransform);
		assert(savedTransform.at("unrelated") == transformOnDisk.at("unrelated"));
		assert(*consumerCfg.getJson() == savedConsumer);
		assert(readConfig(consumerPath) == savedConsumer);

		consumer.transform(nullptr);
		consumerJson.erase("_TransformRigid");
		consumer.setName("withoutTransform");
		transform.setTranslation(7, 8, 9);
		assert(consumer.saveConfig(false));
		assert(consumer.saveConfig(true));
		assert(readConfig(consumerPath) == *consumerCfg.getJson());
		assert(consumerJson.at("name") == "withoutTransform");
		assert(!consumerJson.contains("vT") && !consumerJson.contains("vR"));
		assert(*transformCfg.getJson() == savedTransform);
		assert(readConfig(transformPath) == savedTransform);
		assert(std::remove(consumerPath.c_str()) == 0);
		assert(std::remove(transformPath.c_str()) == 0);
		assert(rmdir(directory) == 0);
	}

	void testPose(void)
	{
		_TransformRigid transform;
		assert(transform.getPoseCombined().matrix().isIdentity());
		assert(transform.getPoseCombinedf().matrix().isIdentity());
		transform.setTranslation(1.0, 2.0, 3.0);
		transform.setAngles(0.2, -0.3, 0.4);
		const Quaterniond expected = Eigen::AngleAxisd(0.4, Vector3d::UnitZ()) *
			Eigen::AngleAxisd(-0.3, Vector3d::UnitY()) * Eigen::AngleAxisd(0.2, Vector3d::UnitX());
		assert(transform.getPose().linear().isApprox(expected.toRotationMatrix()));
		assert(transform.getPose().translation().isApprox(Vector3d(1, 2, 3)));
		transform.setRotation(expected.x() * 2, expected.y() * 2, expected.z() * 2, expected.w() * 2, true);
		assert(transform.getAngles().isApprox(Vector3d(0.2, -0.3, 0.4)));
		assert(transform.getPoseCombinedf().matrix().isApprox(transform.getPose().matrix().cast<float>()));
		const Isometry3d before = transform.getPose();
		transform.setRotation(0, 0, 0, 0);
		transform.setTranslation(std::numeric_limits<double>::quiet_NaN(), 0, 0);
		assert(transform.getPose().matrix().isApprox(before.matrix()));
	}

#ifdef WITH_PROTOCOL
	class TransformReplyTransport : public _JSONbase
	{
	public:
		vector<json> replies;
		bool sendJson(const json &reply) override
		{
			replies.push_back(reply);
			return true;
		}
	};

	void testConsole(void)
	{
		char path[] = "/tmp/openkai-transform-console-XXXXXX";
		const int fd = mkstemp(path);
		assert(fd >= 0);
		close(fd);
		JsonCfg cfg;
		cfg.setJson({{"mount", {{"class", "_TransformRigid"}, {"vT", {0, 0, 0}}, {"vR", {0, 0, 0, 1}}}},
			{"unrelated", "preserve"}});
		assert(cfg.saveToFile(path));
		assert(cfg.readFromFile(path));
		_TransformRigid transform;
		transform.setName("mount");
		transform.setConfig(&cfg, &(*cfg.getJson())["mount"]);
		assert(transform.loadConfig());
		TransformReplyTransport transport;
		// Exercise the same virtual dispatch used by _WSconsole.
		BASE &module = transform;
		const auto command = [&](const string &cmd, const json &config = json()) -> json
		{
			const size_t before = transport.replies.size();
			module.console({{"cmd", cmd}, {"module", "mount"}, {"requestId", "transform-test"},
				{"config", config}}, &transport);
			assert(transport.replies.size() == before + 1);
			const json &reply = transport.replies.back();
			assert(reply.at("cmd") == cmd && reply.at("module") == "mount");
			assert(reply.at("requestId") == "transform-test");
			return reply;
		};
		json reply = command("getConfig");
		assert(reply.at("bSuccess") == true);
		assert(reply.at("config").at("vT") == json({0, 0, 0}));
		assert(reply.at("config").at("vAngle") == json({0, 0, 0}));
		reply = command("setConfig", {{"vT", {"1.5", 2, -3}}, {"vAngle", {0.2, "-0.3", 0.4}}});
		assert(reply.at("bSuccess") == true);
		assert(transform.getTranslation().isApprox(Vector3d(1.5, 2, -3)));
		assert(transform.getAngles().isApprox(Vector3d(0.2, -0.3, 0.4)));
		assert(transform.getPoseCombinedf().matrix().isApprox(transform.getPose().matrix().cast<float>()));
		assert(readConfig(path).at("mount").at("vT") == json({0, 0, 0}));
		// A translation-only edit must preserve a rotation supplied directly as a quaternion.
		transform.setRotation(0, 0, 0.5, 0.5);
		const Quaterniond directRotation = transform.getRotation();
		reply = command("setConfig", {{"vT", {1.5, 2, -3}}});
		assert(reply.at("bSuccess") == true);
		assert(transform.getRotation().coeffs().isApprox(directRotation.coeffs()));
		assert(std::abs(reply.at("config").at("vAngle")[2].get<double>() - std::acos(-1.0) / 2) < 1e-12);
		assert(command("setConfig", {{"vAngle", {0.2, -0.3, 0.4}}}).at("bSuccess") == true);
		const Isometry3d applied = transform.getPose();
		const vector<json> invalid = {json(), json::array(),
			{{"vT", {4, 5, 6}}, {"vAngle", {0, "nan", 0}}},
			{{"vT", {4, 5, 6}}, {"vAngle", {0, 1}}},
			{{"vT", {4, 5, "6junk"}}}, {{"vT", {4, false, 6}}},
			{{"vT", {4, "1e9999", 6}}}, {{"vT", {4, "inf", 6}}},
			{{"vT", {4, 5, 6}}, {"unknown", 7}}};
		for (const json &config : invalid)
		{
			reply = command("setConfig", config);
			assert(reply.at("bSuccess") == false && reply.contains("error"));
			assert(transform.getPose().matrix().isApprox(applied.matrix()));
			assert(reply.at("config").at("vT") == json({1.5, 2, -3}));
		}
		reply = command("setConfig", {{"vT", {7, 8, 9}}});
		assert(reply.at("bSuccess") == true);
		assert(transform.getAngles().isApprox(Vector3d(0.2, -0.3, 0.4)));
		reply = command("saveConfig");
		assert(reply.at("bSuccess") == true);
		const json saved = readConfig(path);
		assertSavedPose(saved.at("mount"), Vector3d(7, 8, 9), transform.getRotation());
		assert(!saved.at("mount").contains("vAngle"));
		assert(saved.at("unrelated") == "preserve");
		JsonCfg reloadedCfg;
		assert(reloadedCfg.readFromFile(path));
		_TransformRigid reloaded;
		reloaded.setName("mount");
		reloaded.setConfig(&reloadedCfg, &(*reloadedCfg.getJson())["mount"]);
		assert(reloaded.loadConfig());
		assert(reloaded.getPose().matrix().isApprox(transform.getPose().matrix()));
		assert(reloaded.getAngles().isApprox(transform.getAngles()));
		const size_t beforeIgnored = transport.replies.size();
		module.console({{"cmd", "unsupported"}}, &transport);
		module.console({{"cmd", 3}}, &transport);
		module.console(json::array(), &transport);
		module.console({{"cmd", "setConfig"}, {"config", {{"vT", {0, 0, 0}}}}}, nullptr);
		assert(transport.replies.size() == beforeIgnored);
		assert(transform.getTranslation().isApprox(Vector3d(7, 8, 9)));
		_TransformRigid unconfigured;
		unconfigured.console({{"cmd", "saveConfig"}}, &transport);
		assert(transport.replies.back().at("bSuccess") == false);
		assert(transport.replies.back().contains("error"));
		assert(std::remove(path) == 0);
	}
#endif

	void addModules(InstanceMgr &instances, JsonCfg &cfg)
	{
		for (auto &entry : cfg.getJson()->items())
		{
			assert(instances.addModule(entry.key(), &cfg, &entry.value()));
		}
		assert(instances.initAll());
	}

	void testTree(bool childrenFirst)
	{
		JsonCfg cfg;
		cfg.setJson({
			{"aChild", {{"class", "_TransformRigid"}, {"vT", {1, 0, 0}}}},
			{"zParent", {{"class", "_TransformRigid"}, {"vT", {10, 0, 0}},
				{"vR", {0, 0, std::sqrt(0.5), std::sqrt(0.5)}}}},
			{"bLeaf", {{"class", "_TransformRigid"}, {"vT", {0, 2, 0}}, {"_TransformRigidParent", "aChild"}}}
		});
		json &j = *cfg.getJson();
		if (childrenFirst)
		{
			j["zParent"]["_TransformRigidChildren"] = {"aChild", "aChild"};
		}
		else
		{
			j["aChild"]["_TransformRigidParent"] = "zParent";
		}
		InstanceMgr instances;
		addModules(instances, cfg);
		assert(instances.linkAll());
		auto *parent = static_cast<_TransformRigid *>(instances.findModule("zParent"));
		auto *child = static_cast<_TransformRigid *>(instances.findModule("aChild"));
		auto *leaf = static_cast<_TransformRigid *>(instances.findModule("bLeaf"));
		assert(child->getPoseCombined().translation().isApprox(Vector3d(10, 1, 0)));
		assert(leaf->getPoseCombined().translation().isApprox(Vector3d(8, 1, 0)));
		parent->setTranslation(20, 0, 0);
		assert(leaf->getPoseCombinedf().translation().isApprox(Vector3f(18, 1, 0)));
		assert(parent->saveConfig(false));
		assert(j["zParent"]["vT"] == json({20, 0, 0}));
	}

	void testInvalidTrees(void)
	{
		const vector<json> configs = {
			{{"a", {{"class", "_TransformRigid"}, {"_TransformRigidParent", "missing"}}}},
			{{"a", {{"class", "_TransformRigid"}, {"_TransformRigidParent", "a"}}}},
			{{"a", {{"class", "_TransformRigid"}, {"_TransformRigidChildren", {"missing"}}}}},
			{{"a", {{"class", "_TransformRigid"}, {"_TransformRigidParent", "b"}}},
			 {"b", {{"class", "_TransformRigid"}, {"_TransformRigidParent", "a"}}}},
			{{"a", {{"class", "_TransformRigid"}, {"_TransformRigidChildren", {"c"}}}},
			 {"b", {{"class", "_TransformRigid"}, {"_TransformRigidChildren", {"c"}}}},
			 {"c", {{"class", "_TransformRigid"}}}}
		};
		for (const json &config : configs)
		{
			JsonCfg cfg;
			cfg.setJson(config);
			InstanceMgr instances;
			addModules(instances, cfg);
			assert(!instances.linkAll());
		}
	}

	void testWorker(void)
	{
		JsonCfg cfg;
		cfg.setJson({{"root", {{"class", "_TransformRigid"}}},
			{"child", {{"class", "_TransformRigid"}, {"thread", {{"FPS", 1}}}}}});
		TransformHarness parent;
		TransformHarness child;
		parent.setName("root");
		child.setName("child");
		parent.setConfig(&cfg, &(*cfg.getJson())["root"]);
		child.setConfig(&cfg, &(*cfg.getJson())["child"]);
		assert(parent.loadConfig());
		assert(child.loadConfig());
		child.parent(parent);
		assert(child.start());
		parent.setTranslation(3, 4, 5);
		assert(child.m_wakeups.load() > 0);
		for (unsigned i = 0; i < 200 && !child.cachedPose().translation().isApprox(Vector3d(3, 4, 5)); ++i)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		assert(child.cachedPose().translation().isApprox(Vector3d(3, 4, 5)));
		child.stop();
	}

	void testFileLinkOrder(void)
	{
		char path[] = "/tmp/openkai-transform-cloud-XXXXXX";
		const int fd = mkstemp(path);
		assert(fd >= 0);
		close(fd);
		GEOMETRY_POINT point;
		point.m_vP = Vector3f(1, 0, 0);
		point.m_tStamp = 1;
		assert(_PCLfile::savePLY(path, {point}));
		JsonCfg cfg;
		cfg.setJson({
			{"aFile", {{"class", "_PCLfile"}, {"vfName", {path}},
				{"_TransformRigid", "zMount"}, {"PCLframeOut", "cloud"}}},
			{"zMount", {{"class", "_TransformRigid"}, {"vT", {0, 2, 0}},
				{"_TransformRigidParent", "zParent"}}},
			{"zParent", {{"class", "_TransformRigid"}, {"vT", {10, 0, 0}}}},
			{"cloud", {{"class", "PCLframe"}}}
		});
		json &j = *cfg.getJson();
		InstanceMgr instances;
		for (const char *name : {"aFile", "zMount", "zParent"})
		{
			assert(instances.addModule(name, &cfg, &j[name]));
		}
		assert(instances.addDataObject("cloud", &cfg, &j["cloud"]));
		assert(instances.initAll());
		assert(instances.linkAll());
		auto *file = static_cast<_PCLfile *>(instances.findModule("aFile"));
		assert(file->start());
		vector<GEOMETRY_POINT> points;
		static_cast<PCLframe *>(instances.findDataObject("cloud"))->get(points);
		assert(points.size() == 1);
		assert(points[0].m_vP.isApprox(Vector3f(11, 2, 0)));
		file->stop();
		assert(std::remove(path) == 0);
	}

	void testPointCloud(void)
	{
		JsonCfg cfg;
		cfg.setJson({
			{"parent", {{"class", "_TransformRigid"}, {"vT", {10, 0, 0}}}},
			{"mount", {{"class", "_TransformRigid"}, {"vT", {0, 2, 0}}, {"_TransformRigidParent", "parent"}}},
			{"in", {{"class", "PCLframe"}}}, {"out", {{"class", "PCLframe"}}},
			{"merge", {{"class", "_PCmerge"}, {"_TransformRigid", "mount"}, {"vPCLframesIn", {"in"}}, {"PCLframeOut", "out"}}}
		});
		json &j = *cfg.getJson();
		InstanceMgr instances;
		assert(instances.addModule("mount", &cfg, &j["mount"]));
		assert(instances.addModule("parent", &cfg, &j["parent"]));
		assert(instances.addDataObject("in", &cfg, &j["in"]));
		assert(instances.addDataObject("out", &cfg, &j["out"]));
		assert(instances.initAll());
		assert(instances.linkAll());
		MergeHarness merge;
		merge.setName("merge");
		merge.setConfig(&cfg, &j["merge"]);
		assert(merge.loadConfig());
		assert(merge.link(&instances));
		auto *input = static_cast<PCLframe *>(instances.findDataObject("in"));
		auto *output = static_cast<PCLframe *>(instances.findDataObject("out"));
		GEOMETRY_POINT point;
		point.m_vP = Vector3f(1, 0, 0);
		point.m_tStamp = 1;
		input->set({point}, 1);
		assert(merge.start());
		for (unsigned i = 0; i < 200 && output->getTstamp() != input->getTstamp(); ++i)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		merge.join();
		vector<GEOMETRY_POINT> points;
		output->get(points);
		assert(points.size() == 1);
		assert(points[0].m_vP.isApprox(Vector3f(11, 2, 0)));
		j["merge"]["_TransformRigid"] = "missing";
		assert(!merge.link(&instances));
		j["merge"].erase("_TransformRigid");
		assert(merge.link(&instances));
		input->set({point}, 2);
		assert(merge.start());
		for (unsigned i = 0; i < 200 && output->getTstamp() != input->getTstamp(); ++i)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		merge.join();
		output->get(points);
		assert(points[0].m_vP.isApprox(point.m_vP));
	}
}

int main()
{
	testPose();
#ifdef WITH_PROTOCOL
	testConsole();
#endif
#if defined(WITH_VISION) && defined(USE_OPENCV)
	testRGBDTransform();
#endif
	testTree(false);
	testTree(true);
	testInvalidTrees();
	testWorker();
	testPointCloud();
	testFileLinkOrder();
	testConsumerConfigIsolation<_PCmerge>("_PCmerge", {{"rVoxel", 0.25}});
#ifdef WITH_SLAM
	testConsumerConfigIsolation<_SLAMbase>("_SLAMbase", {{"bAutoStart", false}, {"tConfidenceTimeoutNs", 1234}});
#endif
#ifdef WITH_SENSOR
	testConsumerConfigIsolation<_Airy>("_Airy", {{"bReflectivity", false}});
	testConsumerConfigIsolation<_E1R>("_E1R", {{"bReflectivity", false}});
	testConsumerConfigIsolation<_Livox2>("_Livox2", {{"lvxIP", "127.0.0.2"},
		{"bIMUpreview", true}, {"bIMUstab", true}}, {{"bIMUstab", false}});
#endif
	std::cout << "Rigid transform tests passed\n";
}
