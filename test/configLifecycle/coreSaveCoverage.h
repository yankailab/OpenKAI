#ifndef OpenKAI_test_configLifecycle_coreSaveCoverage_H_
#define OpenKAI_test_configLifecycle_coreSaveCoverage_H_

#include "src/Base/BASE.h"
#ifdef WITH_CONTROL
#include "src/Control/PID.h"
#endif
#ifdef WITH_IO
#include "src/IO/_ADIObase.h"
#include "src/IO/_WebSocket.h"
#endif
#ifdef WITH_PROTOCOL
#include "src/Protocol/_JSONbase.h"
#include "src/Protocol/_Xbee.h"
#endif
#ifdef WITH_SENSOR
#include "src/Sensor/_IMUbase.h"
#endif
#if defined(WITH_NAVIGATION) && defined(WITH_PROTOCOL)
#include "src/Navigation/_RTCMcast.h"
#endif

#include <cassert>
#include <fstream>

namespace kai
{
	static json readCoverageConfig(const string &path)
	{
		std::ifstream input(path);
		assert(input.good());
		return json::parse(input)["module"];
	}

	static void bindCoverageConfig(BASE &module, ModuleMgr &manager, JsonCfg &config,
		const string &path, const json &settings)
	{
		std::ofstream output(path);
		output << json({{"module", settings}, {"sibling", "keep"}}).dump(2);
		output.close();
		assert(output.good());
		assert(config.readFromFile(path));
		module.setModuleMgr(&manager);
		module.setName("module");
		module.setConfig(&config, &(*config.getJson())["module"]);
		assert(module.loadConfig());
	}

#ifdef WITH_IO
	class PortSaveProbe : public _ADIObase
	{
	public:
		void changePort(void)
		{
			m_vPort[0].m_addr = 23;
			m_vPort[0].m_vR = 2.5f;
			assert(writeA(0, 4.5f));
		}
	};
#endif

#ifdef WITH_PROTOCOL
	class JsonSaveProbe : public _JSONbase
	{
	public:
		void changeSettings(void)
		{
			m_ieSendHB.init(75000000);
			m_msgFinishSend = "\r\n";
			m_msgFinishRecv = "\n";
			m_pTr->setTargetFPS(7);
		}
	};

	class XbeeSaveProbe : public _Xbee
	{
	public:
		uint64_t address(void)
		{
			return m_myAddr;
		}
	};
#endif

#ifdef WITH_SENSOR
	class ImuSaveProbe : public _IMUbase
	{
	public:
		void changeSettings(void)
		{
			m_nIMUdqMax = 321;
			m_tIMUpairToleranceNs = 9876;
			m_bFusion = true;
			m_pTstream->setTargetFPS(44);
		}
	};
#endif

#if defined(WITH_NAVIGATION) && defined(WITH_PROTOCOL)
	class RtcmSaveProbe : public _RTCMcast
	{
	public:
		void changeSettings(void)
		{
			m_vMsg[0].m_ieSend.init(250000000);
			m_vMsg[0].m_tOutRecv.setTout(1750000000);
			m_vMsg[0].m_bSendOnceOnly = true;
		}

		void checkSettings(void)
		{
			assert(m_vMsg.size() == 1);
			assert(m_vMsg[0].m_ieSend.m_tInterval == 250000000);
			assert(m_vMsg[0].m_tOutRecv.m_tOut == 1750000000);
		}
	};
#endif

	static void testCoreSaveCoverage(void)
	{
		ModuleMgr manager;
#ifdef WITH_CONTROL
		{
			JsonCfg config;
			PID pid;
			bindCoverageConfig(pid, manager, config, "pid-save.json", {{"class", "PID"}, {"custom", "keep"}});
			pid.setP(1.5f);
			pid.setI(0.25f);
			pid.setImax(3.0f);
			pid.setD(0.5f);
			pid.setRangeIn(Vector2f(-5, 5));
			pid.setRangeOut(Vector2f(-10, 10));
			assert(pid.saveConfig(true));
			const json saved = readCoverageConfig("pid-save.json");
			assert(saved["P"] == 1.5 && saved["I"] == 0.25);
			assert(saved["Imax"] == 3.0 && saved["D"] == 0.5);
			assert(saved["vRin"] == json({-5, 5}));
			assert(saved["vRout"] == json({-10, 10}));
			assert(saved["custom"] == "keep");
			JsonCfg reloaded;
			PID copy;
			bindCoverageConfig(copy, manager, reloaded, "pid-copy.json", saved);
			assert(copy.getP() == 1.5f);
			assert(copy.getRangeOut().isApprox(Vector2f(-10, 10)));
		}
#endif
#ifdef WITH_IO
		{
			JsonCfg config;
			PortSaveProbe ports;
			bindCoverageConfig(ports, manager, config, "ports-save.json", {{"class", "_ADIObase"},
				{"ports", json::array({nullptr, {{"bDigital", false}, {"addr", 2}, {"custom", "keep"}}})}});
			ports.changePort();
			(*config.getJson())["module"]["ports"][1].erase("bDigital");
			assert(ports.saveConfig(true));
			const json saved = readCoverageConfig("ports-save.json");
			assert(saved["ports"][0].is_null());
			assert(saved["ports"][1]["bDigital"] == false);
			assert(saved["ports"][1]["addr"] == 23);
			assert(saved["ports"][1]["vW"] == 4.5);
			assert(saved["ports"][1]["vR"] == 2.5);
			assert(saved["ports"][1]["custom"] == "keep");
		}
		{
			JsonCfg config;
			_WebSocket socket;
			bindCoverageConfig(socket, manager, config, "socket-save.json", {{"class", "_WebSocket"}});
			assert(socket.saveConfig(true));
			const json saved = readCoverageConfig("socket-save.json");
			assert(saved["nPacket"] == 1024 && saved["nPbuffer"] == 512);
			JsonCfg reloaded;
			_WebSocket copy;
			bindCoverageConfig(copy, manager, reloaded, "socket-copy.json", saved);
			assert(copy.getPacketFIFOw()->m_nP == 1024);
			assert(copy.getPacketFIFOr()->m_nP == 1024);
			assert(copy.getPacketFIFOw()->m_pP[0].m_nB == 512);
			assert(copy.getPacketFIFOr()->m_pP[0].m_nB == 512);
		}
#endif
#ifdef WITH_PROTOCOL
		{
			JsonCfg config;
			JsonSaveProbe transport;
			bindCoverageConfig(transport, manager, config, "transport-save.json", {{"class", "_JSONbase"},
				{"threadR", {{"custom", "keep"}}}, {"_IObase", "keep-link"}});
			transport.changeSettings();
			assert(transport.saveConfig(true));
			const json saved = readCoverageConfig("transport-save.json");
			assert(saved["ieSendHB"] == 75000000);
			assert(saved["msgFinishSend"] == "\r\n");
			assert(saved["msgFinishRecv"] == "\n");
			assert(saved["threadR"]["FPS"] == 7);
			assert(saved["threadR"]["custom"] == "keep");
			assert(saved["_IObase"] == "keep-link");
		}
		{
			JsonCfg config;
			XbeeSaveProbe radio;
			bindCoverageConfig(radio, manager, config, "radio-save.json", {{"class", "_Xbee"}, {"myAddr", "fedcba9876543210"}});
			assert(radio.address() == 0xfedcba9876543210ULL);
			(*config.getJson())["module"].erase("myAddr");
			assert(radio.saveConfig(true));
			const json saved = readCoverageConfig("radio-save.json");
			assert(saved["myAddr"] == "fedcba9876543210");
			JsonCfg reloaded;
			XbeeSaveProbe copy;
			bindCoverageConfig(copy, manager, reloaded, "radio-copy.json", saved);
			assert(copy.address() == radio.address());
		}
#endif
#ifdef WITH_SENSOR
		{
			JsonCfg config;
			ImuSaveProbe imu;
			bindCoverageConfig(imu, manager, config, "imu-save.json", {{"class", "_IMUbase"}, {"threadStream", {{"custom", "keep"}}}});
			imu.changeSettings();
			assert(imu.saveConfig(true));
			const json saved = readCoverageConfig("imu-save.json");
			assert(saved["nIMUdqMax"] == 321);
			assert(saved["tIMUpairToleranceNs"] == 9876);
			assert(saved["bFusion"] == true);
			assert(saved["threadStream"]["FPS"] == 44);
			assert(saved["threadStream"]["custom"] == "keep");
		}
#endif
#if defined(WITH_NAVIGATION) && defined(WITH_PROTOCOL)
		{
			JsonCfg config;
			RtcmSaveProbe rtcm;
			bindCoverageConfig(rtcm, manager, config, "rtcm-save.json", {{"class", "_RTCMcast"},
				{"RTCMmsg", {{"station", {{"ID", 1005}, {"custom", "keep"}}}}}});
			rtcm.changeSettings();
			assert(rtcm.saveConfig(true));
			const json saved = readCoverageConfig("rtcm-save.json");
			assert(saved["RTCMmsg"]["station"]["ieSendSec"] == 0.25);
			assert(saved["RTCMmsg"]["station"]["tOutSec"] == 1.75);
			assert(saved["RTCMmsg"]["station"]["custom"] == "keep");
			JsonCfg reloaded;
			RtcmSaveProbe copy;
			bindCoverageConfig(copy, manager, reloaded, "rtcm-copy.json", saved);
			copy.checkSettings();
		}
#endif
	}
}
#endif
