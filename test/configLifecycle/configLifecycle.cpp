#include "coreSaveCoverage.h"
#include "visionSaveCoverage.h"
#include "src/Base/_ModuleBase.h"
#include "src/Module/ModuleMgr.h"

#ifdef WITH_STATE
#include "src/State/_StateControl.h"
#endif

#ifdef WITH_UNIVERSE
#include "src/Universe/Geometry/PointCloud/Pipeline/_PCtransform.h"
#include "src/Universe/Grid/_SelectableOctGrid.h"
#endif

#if defined(WITH_VISION) && defined(USE_SCEPTER_SDK)
#include "src/Vision/RGBD/_Scepter.h"
#endif

#if defined(WITH_VISION) && defined(USE_ORBBEC)
#include "src/Vision/RGBD/_Orbbec.h"
#endif

#if defined(WITH_SLAM) && defined(USE_GLIM) && defined(WITH_PROTOCOL)
#include "src/SLAM/_GLIM.h"
#endif

#ifdef WITH_PROTOCOL
#include "src/Protocol/_JSONbase.h"
#endif

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace kai
{

	class ThreadModule : public _ModuleBase
	{
	public:
		_Thread *thread(void)
		{
			return m_pT;
		}
	};

#ifdef WITH_PROTOCOL
	class CaptureTransport : public _JSONbase
	{
	public:
		bool sendJson(const json &j) override
		{
			m_reply = j;
			return true;
		}

		json m_reply;
	};

#endif

	static void writeJson(const string &path, const json &j)
	{
		std::ofstream output(path);
		output << j.dump(2);
		output.close();
		assert(output.good());
	}

	static json readJson(const string &path)
	{
		std::ifstream input(path);
		assert(input.good());
		return json::parse(input);
	}

	static json moduleConfig(const string &type)
	{
		return {{"class", type}, {"bON", 1}, {"custom", {{"keep", true}}},
			{"thread", {{"FPS", 17}, {"threadCustom", "keep"}}}};
	}

	static json launchConfig(void)
	{
		return {{"APP", {{"class", "ModuleMgr"}, {"bStdErr", false}}},
			{"untouched", {{"class", "Destimator"}, {"bON", 0}, {"custom", 123}}},
			{"description", "Preserve settings outside the saved module"}};
	}

	static void initialize(ModuleMgr &manager, const string &path)
	{
		assert(manager.loadJsonFiles(path));
		assert(manager.createAll());
		assert(manager.initAll());
		assert(manager.linkAll());
	}

	template <typename T>
	static T *findModule(ModuleMgr &manager, const string &name)
	{
		BASE *pBase = static_cast<BASE *>(manager.findModule(name));
		T *pModule = dynamic_cast<T *>(pBase);
		assert(pModule != nullptr);
		return pModule;
	}

	static void testManager(void)
	{
		std::cout << "Manager create/load/link and owner lookup" << std::endl;
		json main = launchConfig();
		main["APP"]["vInclude"] = {"manager-include.json"};
		main["primary"] = moduleConfig("Destimator");
		json included = {{"secondary", moduleConfig("Destimator")}};
		writeJson("manager.json", main);
		writeJson("manager-include.json", included);

		ModuleMgr manager;
		initialize(manager, "manager.json");
		assert(findModule<BASE>(manager, "primary")->getClass() == "Destimator");
		assert(findModule<BASE>(manager, "secondary")->getName() == "secondary");
		assert(manager.findModule("untouched") == nullptr);
		assert(manager.findJsonCfg("primary") != manager.findJsonCfg("secondary"));
		assert(manager.findJson("secondary") ==
			&(*manager.findJsonCfg("secondary")->getJson())["secondary"]);

		BASE *pPrimary = findModule<BASE>(manager, "primary");
		manager.findJson("primary")->erase("class");
		assert(pPrimary->BASE::saveConfig(false));
		assert((*manager.findJson("primary"))["class"] == "Destimator");
		assert(readJson("manager.json") == main);
		assert(pPrimary->BASE::saveConfig(true));
		assert(readJson("manager.json")["primary"]["name"] == "primary");

		assert(std::filesystem::remove("manager.json"));
		assert(std::filesystem::create_directory("manager.json"));
		assert(pPrimary->BASE::saveConfig(false));
		assert(!pPrimary->BASE::saveConfig(true));

		BASE unbound;
		assert(!unbound.loadConfig());
		assert(!unbound.link());
		assert(!unbound.saveConfig(false));
		assert(!unbound.saveConfig(true));
	}

	static void testNestedThreads(void)
	{
		std::cout << "Nested thread ownership, defaults, save and reload" << std::endl;
		json main = launchConfig();
		main["APP"]["vInclude"] = {"threads-include.json"};
		json included = {{"configured", moduleConfig("_ModuleBase")},
			{"defaults", {{"class", "_ModuleBase"}}}, {"sibling", "keep"}};
		writeJson("threads.json", main);
		writeJson("threads-include.json", included);

		ModuleMgr manager;
		assert(manager.loadJsonFiles("threads.json"));
		ThreadModule *pConfigured = new ThreadModule();
		ThreadModule *pDefaults = new ThreadModule();
		assert(manager.addModule(pConfigured, "configured"));
		assert(manager.addModule(pDefaults, "defaults"));
		assert(manager.initAll());
		assert(manager.linkAll());
		assert(pConfigured->thread()->getTargetFPS() == 17);
		assert(pDefaults->thread()->getTargetFPS() == DEFAULT_FPS);
		assert((*manager.findJson("defaults"))["thread"].is_object());

		pConfigured->thread()->setTargetFPS(41);
		assert(pConfigured->saveConfig(false));
		assert((*manager.findJson("configured"))["name"] == "configured");
		assert((*manager.findJson("configured"))["thread"]["FPS"] == 41);
		assert(readJson("threads-include.json") == included);
		assert(readJson("threads.json") == main);
		assert(pConfigured->saveConfig(true));
		json saved = readJson("threads-include.json");
		assert(saved["configured"]["thread"]["FPS"] == 41);
		assert(saved["configured"]["thread"]["threadCustom"] == "keep");
		assert(saved["configured"]["custom"] == included["configured"]["custom"]);
		assert(saved["sibling"] == included["sibling"]);
		assert(readJson("threads.json") == main);

		pConfigured->thread()->setTargetFPS(53);
		assert(pConfigured->thread()->saveConfig(true));
		assert(readJson("threads-include.json")["configured"]["thread"]["FPS"] == 53);
		assert(pConfigured->loadConfig());
		assert(pConfigured->link());
		assert(pConfigured->thread()->getTargetFPS() == 53);

		ModuleMgr reloaded;
		assert(reloaded.loadJsonFiles("threads.json"));
		ThreadModule *pReloaded = new ThreadModule();
		assert(reloaded.addModule(pReloaded, "configured"));
		assert(reloaded.initAll());
		assert(reloaded.linkAll());
		assert(pReloaded->thread()->getTargetFPS() == 53);

		pDefaults->thread()->setTargetFPS(29);
		assert(pDefaults->saveConfig(true));
		saved = readJson("threads-include.json");
		assert(saved["configured"]["thread"]["FPS"] == 53);
		assert(saved["defaults"]["thread"]["FPS"] == 29);
		assert(saved["sibling"] == included["sibling"]);

		pConfigured->thread()->setTargetFPS(61);
		assert(pConfigured->saveConfig(true));
		saved = readJson("threads-include.json");
		assert(saved["configured"]["thread"]["FPS"] == 61);
		assert(saved["defaults"]["thread"]["FPS"] == 29);
		assert(saved["sibling"] == included["sibling"]);

		// Replacing this temporary destination with a directory forces a write error.
		assert(std::filesystem::remove("threads-include.json"));
		assert(std::filesystem::create_directory("threads-include.json"));
		pConfigured->thread()->setTargetFPS(67);
		assert(pConfigured->saveConfig(false));
		assert((*manager.findJson("configured"))["thread"]["FPS"] == 67);
		pConfigured->thread()->setTargetFPS(71);
		assert(pConfigured->thread()->saveConfig(false));
		assert((*manager.findJson("configured"))["thread"]["FPS"] == 71);
		assert(std::filesystem::is_directory("threads-include.json"));
		assert(!pConfigured->saveConfig(true));
		assert(!pConfigured->thread()->saveConfig(true));
	}

	static void testBufferedWriteFailure(void)
	{
		if (!std::filesystem::exists("/dev/full"))
		{
			return;
		}

		std::cout << "Buffered write failure propagation" << std::endl;
		JsonCfg config;
		config.setJson({{"small", "buffered JSON"}});
		assert(!config.saveToFile("/dev/full"));
	}

#ifdef WITH_STATE
	static void testNestedStates(void)
	{
		std::cout << "Embedded state binding without redundant names or classes" << std::endl;
		json main = launchConfig();
		main["controller"] = moduleConfig("_StateControl");
		main["controller"]["start"] = "ready";
		main["controller"]["states"] = {{"ready", {{"next", "done"}}},
			{"done", json::object()}};
		writeJson("states.json", main);

		ModuleMgr manager;
		initialize(manager, "states.json");
		_StateControl *pController = findModule<_StateControl>(manager, "controller");
		StateBase *pState = pController->getCurrentState();
		assert(pState != nullptr);
		assert(pState->getName() == "ready");
		assert(pState->getClass() == "StateBase");
		assert(pState->getNext() == "done");
		(*manager.findJson("controller"))["states"]["ready"]["next"] = "ready";
		assert(pState->loadConfig());
		assert(pState->getNext() == "ready");
		assert(pController->saveConfig(true));
		assert(readJson("states.json")["controller"]["states"]["ready"]["next"] == "ready");
	}
#endif

#ifdef WITH_UNIVERSE
	static void testGeometry(void)
	{
		std::cout << "Geometry inheritance, flat config and transform round trip" << std::endl;
		json main = launchConfig();
		main["frame"] = moduleConfig("_ReferenceFrame");
		main["frame"]["vPos"] = {1.0, 2.0, 3.0};
		main["transform"] = moduleConfig("_PCtransform");
		main["transform"]["nP"] = 16;
		main["transform"]["_PointCloud"] = "cloud";
		main["cloud"] = moduleConfig("_PointCloud");
		main["cloud"]["nP"] = 16;
		main["grid"] = moduleConfig("_SelectableOctGrid");
		main["grid"]["nMaxLevel"] = 4;
		main["grid"]["nMaxCells"] = 8;
		main["grid"]["vSelectedCells"] = {"00000000000000000000000000000000"};
		writeJson("geometry.json", main);

		ModuleMgr manager;
		initialize(manager, "geometry.json");
		_ReferenceFrame *pFrame = findModule<_ReferenceFrame>(manager, "frame");
		_PCtransform *pTransform = findModule<_PCtransform>(manager, "transform");
		_SelectableOctGrid *pGrid = findModule<_SelectableOctGrid>(manager, "grid");
		assert(pFrame->getPos().isApprox(Vector3d(1.0, 2.0, 3.0)));
		pFrame->setPos(4.0, 5.0, 6.0);
		assert(pFrame->saveConfig(true));
		pTransform->setPos(7.0, 8.0, 9.0);
		Eigen::Matrix4d transform = Eigen::Matrix4d::Identity();
		transform.block<3, 1>(0, 3) = Vector3d(10.0, 11.0, 12.0);
		pTransform->setTranslationMatrix(transform);
		const json beforeTransform = readJson("geometry.json");
		assert(pTransform->saveConfig(false));
		assert((*manager.findJson("transform"))["vPos"] == json({7.0, 8.0, 9.0}));
		assert((*manager.findJson("transform"))["nP"] == 16);
		assert((*manager.findJson("transform"))["mT"][3] == 10.0);
		assert(readJson("geometry.json") == beforeTransform);
		assert(pTransform->saveConfig(true));
		assert(pGrid->saveConfig(true));

		json saved = readJson("geometry.json");
		assert(saved["frame"]["vPos"] == json({4.0, 5.0, 6.0}));
		assert(saved["transform"]["vPos"] == json({7.0, 8.0, 9.0}));
		assert(saved["transform"]["mT"][3] == 10.0);
		assert(!saved["transform"].contains("_PCtransform"));
		assert(!saved["transform"].contains("_ReferenceFrame"));
		assert(saved["transform"]["thread"]["FPS"] == 17);
		assert(saved["transform"]["custom"] == main["transform"]["custom"]);
		assert(saved["untouched"] == main["untouched"]);
		assert(saved["grid"]["vSelectedCells"] == main["grid"]["vSelectedCells"]);

		ModuleMgr reloaded;
		initialize(reloaded, "geometry.json");
		assert(findModule<_ReferenceFrame>(reloaded, "frame")->getPos().isApprox(Vector3d(4, 5, 6)));
		_PCtransform *pReloaded = findModule<_PCtransform>(reloaded, "transform");
		assert(pReloaded->getPos().isApprox(Vector3d(7, 8, 9)));
		assert(pReloaded->getTranslationMatrix().isApprox(transform));
		assert(findModule<_SelectableOctGrid>(reloaded, "grid")->getSelectedCells().size() == 1);
	}
#endif

#if defined(WITH_VISION) && defined(USE_SCEPTER_SDK)
	static void testScepter(void)
	{
		std::cout << "Scepter camera controls and threadPP round trip" << std::endl;
		json main = launchConfig();
		main["camera"] = moduleConfig("_Scepter");
		main["camera"]["threadPP"] = {{"FPS", 13}, {"custom", "keep"}};
		main["camera"]["bRGB"] = false;
		main["camera"]["bPCLrgb"] = false;
		main["camera"]["scFrameRate"] = 15;
		writeJson("scepter.json", main);

		ModuleMgr manager;
		initialize(manager, "scepter.json");
		_Scepter *pCamera = findModule<_Scepter>(manager, "camera");
		assert(pCamera->getCamCtrl().m_frameRate == 15);
		ScCtrl control = pCamera->getCamCtrl();
		control.m_frameRate = 20;
		assert(pCamera->setCamCtrl(control));
#ifdef WITH_PROTOCOL
		CaptureTransport transport;
		pCamera->console({{"cmd", "saveConfig"}, {"requestId", "camera-save"}}, &transport);
		assert(transport.m_reply["bSuccess"] == true);
		assert(transport.m_reply["requestId"] == "camera-save");
#else
		assert(pCamera->saveConfig(true));
#endif
		json saved = readJson("scepter.json");
		assert(saved["camera"]["scFrameRate"] == 20);
		assert(saved["camera"]["threadPP"]["FPS"] == 13);
		assert(saved["camera"]["threadPP"]["custom"] == "keep");
		assert(saved["camera"]["custom"] == main["camera"]["custom"]);
		assert(saved["untouched"] == main["untouched"]);

		ModuleMgr reloaded;
		initialize(reloaded, "scepter.json");
		assert(findModule<_Scepter>(reloaded, "camera")->getCamCtrl().m_frameRate == 20);
	}
#endif

#if defined(WITH_VISION) && defined(USE_ORBBEC)
	static void testOrbbec(void)
	{
		std::cout << "Orbbec optional controls and threadPP defaults round trip" << std::endl;
		json main = launchConfig();
		main["camera"] = moduleConfig("_Orbbec");
		main["camera"]["bRGB"] = false;
		main["camera"]["bPCLrgb"] = false;
		main["camera"]["OB_PROP_LASER_BOOL"] = false;
		writeJson("orbbec.json", main);

		ModuleMgr manager;
		initialize(manager, "orbbec.json");
		_Orbbec *pCamera = findModule<_Orbbec>(manager, "camera");
		assert(pCamera->getCamCtrl().m_laser == false);
		// The setter caches the request even though this test has no device open.
		pCamera->setLaser(true);
		assert(pCamera->getCamCtrl().m_laser == true);
#ifdef WITH_PROTOCOL
		CaptureTransport transport;
		pCamera->console({{"cmd", "saveConfig"}, {"requestId", "camera-save"}}, &transport);
		assert(transport.m_reply["bSuccess"] == true);
		assert(transport.m_reply["requestId"] == "camera-save");
#else
		assert(pCamera->saveConfig(true));
#endif
		json saved = readJson("orbbec.json");
		assert(saved["camera"]["OB_PROP_LASER_BOOL"] == true);
		assert(saved["camera"]["threadPP"].is_object());
		assert(saved["camera"]["custom"] == main["camera"]["custom"]);
		assert(saved["untouched"] == main["untouched"]);

		ModuleMgr reloaded;
		initialize(reloaded, "orbbec.json");
		assert(findModule<_Orbbec>(reloaded, "camera")->getCamCtrl().m_laser == true);
	}
#endif

#if defined(WITH_SLAM) && defined(USE_GLIM) && defined(WITH_PROTOCOL)

	static void testGlim(const string &root)
	{
		std::cout << "GLIM parameters, inherited pose and launch config round trip" << std::endl;
		json main = launchConfig();
		main["slam"] = moduleConfig("_GLIM");
		main["slam"]["configPath"] = root + "/jsonCfg/glim_orbbec";
		main["slam"]["_PointCloud"] = "cloud";
		main["slam"]["parameters"] = {{"nMinPoints", 64}, {"preprocess", {{"distanceFar", 17.0}}}};
		main["cloud"] = moduleConfig("_PointCloud");
		main["cloud"]["nP"] = 16;
		writeJson("glim.json", main);

		ModuleMgr manager;
		initialize(manager, "glim.json");
		_GLIM *pSlam = findModule<_GLIM>(manager, "slam");
		CaptureTransport transport;
		pSlam->console({{"cmd", "getConfig"}}, &transport);
		assert(transport.m_reply["bSuccess"] == true);
		assert(transport.m_reply["config"]["nMinPoints"] == 64);
		pSlam->console({{"cmd", "setConfig"}, {"config", {{"nMinPoints", 80}}}}, &transport);
		assert(transport.m_reply["bSuccess"] == true);
		pSlam->setPos(3.0, 4.0, 5.0);
		assert(pSlam->saveConfig(true));
		json saved = readJson("glim.json");
		assert(saved["slam"]["parameters"]["nMinPoints"] == 80);
		assert(saved["slam"]["parameters"]["preprocess"]["distanceFar"] == 17.0);
		assert(saved["slam"]["vPos"] == json({3.0, 4.0, 5.0}));
		assert(saved["slam"]["custom"] == main["slam"]["custom"]);
		assert(saved["untouched"] == main["untouched"]);

		ModuleMgr reloaded;
		initialize(reloaded, "glim.json");
		_GLIM *pReloaded = findModule<_GLIM>(reloaded, "slam");
		pReloaded->console({{"cmd", "getConfig"}}, &transport);
		assert(transport.m_reply["config"]["nMinPoints"] == 80);
		assert(transport.m_reply["config"]["preprocess"]["distanceFar"] == 17.0);
		assert(pReloaded->getPos().isApprox(Vector3d(3, 4, 5)));
	}
#endif
}

int main(int argc, char **argv)
{
	assert(argc == 2);
	kai::testCoreSaveCoverage();
	kai::testVisionSaveCoverage();
	kai::testManager();
	kai::testNestedThreads();
	kai::testBufferedWriteFailure();
#ifdef WITH_STATE
	kai::testNestedStates();
#endif
#ifdef WITH_UNIVERSE
	kai::testGeometry();
#endif
#if defined(WITH_VISION) && defined(USE_SCEPTER_SDK)
	kai::testScepter();
#endif
#if defined(WITH_VISION) && defined(USE_ORBBEC)
	kai::testOrbbec();
#endif
#if defined(WITH_SLAM) && defined(USE_GLIM) && defined(WITH_PROTOCOL)
	kai::testGlim(argv[1]);
#endif
	std::cout << "PASS: configuration lifecycle checks" << std::endl;
	return 0;
}
