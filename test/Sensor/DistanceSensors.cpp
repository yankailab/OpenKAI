#ifdef OPENKAI_DISTANCE_SENSOR_TEST

#include "../../src/Sensor/Distance/_BenewakeTF.h"
#include "../../src/Sensor/Distance/_TOFsense.h"
#include "../../src/Sensor/Distance/_LeddarVu.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

namespace
{
	using namespace kai;
	using namespace std::chrono_literals;

	std::mutex responseMutex;
	vector<uint16_t> registerResponse;
	vector<uint8_t> fastResponse;

	class LeddarHarness : public _LeddarVu
	{
	public:
		void useSimulatedDevice(void)
		{
			// A real, unconnected context avoids opening hardware. The three
			// data transfer functions are replaced by the linker's --wrap option.
			m_pMb = modbus_new_rtu("unused", 115200, 'N', 8, 1);
			assert(m_pMb);
		}
	};

	struct Streams
	{
		JsonCfg m_cfg;
		InstanceMgr m_instances;
		BytePacketStream *m_pBytesout = nullptr;
		PCLframe *m_pPCLin = nullptr;

		Streams(void)
		{
			m_cfg.setJson({
				{"bytes", {{"class", "BytePacketStream"}}},
				{"points", {{"class", "PCLframe"}}},
			});
			json &j = *m_cfg.getJson();
			assert(m_instances.addDataObject("bytes", &m_cfg, &j["bytes"]));
			assert(m_instances.addDataObject("points", &m_cfg, &j["points"]));
			assert(m_instances.initAll());
			m_pBytesout = static_cast<BytePacketStream *>(m_instances.findDataObject("bytes"));
			m_pPCLin = static_cast<PCLframe *>(m_instances.findDataObject("points"));
		}

		void configure(_ModuleBase &sensor, const string &name, json settings = json::object())
		{
			settings["class"] = name;
			settings["BytePacketStreamIn"] = "bytes";
			settings["PCLframeOut"] = "points";
			settings["thread"] = {{"FPS", 200}};
			json &j = (*m_cfg.getJson())["sensor"];
			j = settings;
			sensor.setConfig(&m_cfg, &j);
			sensor.setName(name);
			assert(sensor.loadConfig());
			assert(sensor.link(&m_instances));
			assert(sensor.saveConfig(false));
			for (auto it = settings.begin(); it != settings.end(); ++it)
			{
				if (it.key() == "vRange")
				{
					for (size_t i = 0; i < 2; i++)
					{
						assert(std::abs(j["vRange"][i].get<double>() - it.value()[i].get<double>()) < 1e-6);
					}
				}
				else if (it.key() != "thread")
				{
					assert(j[it.key()] == it.value());
				}
			}
		}

		void send(const vector<uint8_t> &bytes)
		{
			m_pBytesout->add({{bytes, getTns()}});
		}

		vector<GEOMETRY_POINT> readAfter(uint64_t stamp)
		{
			vector<GEOMETRY_POINT> points;
			for (int i = 0; i < 1000; i++)
			{
				if (m_pPCLin->get(points) > stamp)
				{
					return points;
				}
				std::this_thread::sleep_for(1ms);
			}
			assert(false && "sensor did not publish a frame");
			return points;
		}

		void expectUnchanged(uint64_t stamp)
		{
			std::this_thread::sleep_for(30ms);
			assert(m_pPCLin->getTstamp() == stamp);
		}
	};

	void checksum(vector<uint8_t> &bytes)
	{
		bytes.back() = 0;
		for (size_t i = 0; i + 1 < bytes.size(); i++)
		{
			bytes.back() += bytes[i];
		}
	}

	vector<uint8_t> benewakePacket(uint16_t distance, uint16_t amplitude)
	{
		vector<uint8_t> bytes = {0x59, 0x59, uint8_t(distance), uint8_t(distance >> 8),
			uint8_t(amplitude), uint8_t(amplitude >> 8), 0, 0, 0};
		checksum(bytes);
		return bytes;
	}

	vector<uint8_t> tofPacket(uint8_t id, int32_t distance, uint16_t amplitude, uint8_t status = 0)
	{
		uint32_t raw = static_cast<uint32_t>(distance);
		vector<uint8_t> bytes = {0x57, 0, 0, id, 0, 0, 0, 0,
			uint8_t(raw), uint8_t(raw >> 8), uint8_t(raw >> 16), status,
			uint8_t(amplitude), uint8_t(amplitude >> 8), 0, 0};
		checksum(bytes);
		return bytes;
	}

	void checkPoint(const GEOMETRY_POINT &point, float distance, float amplitude, float x, float y)
	{
		assert(point.m_tStamp != 0);
		assert(std::abs(point.m_vP.norm() - distance) < 1e-5f);
		assert(std::abs(point.m_vP.x() - x) < 1e-5f);
		assert(std::abs(point.m_vP.y() - y) < 1e-5f);
		assert(point.m_vP.z() == 0);
		assert(point.m_vC == Vector3f(amplitude, 0, 0));
	}

	void testBenewake(void)
	{
		Streams streams;
		_BenewakeTF sensor;
		streams.configure(sensor, "_BenewakeTF", {{"vRange", {0.1, 10.0}}});
		assert(sensor.start());
		vector<uint8_t> bytes = benewakePacket(250, 1234);
		streams.send(vector<uint8_t>(bytes.begin(), bytes.begin() + 4));
		streams.expectUnchanged(0);
		streams.send(vector<uint8_t>(bytes.begin() + 4, bytes.end()));
		auto points = streams.readAfter(0);
		assert(points.size() == 1);
		checkPoint(points[0], 2.5, 1234, 2.5, 0);
		uint64_t stamp = streams.m_pPCLin->getTstamp();
		assert(points[0].m_tStamp == stamp);

		bytes.back()++;
		streams.send(bytes);
		streams.expectUnchanged(stamp);
		streams.send(benewakePacket(1000, 456));
		points = streams.readAfter(stamp);
		assert(points.size() == 1 && points[0].m_tStamp == 0);
		assert(points[0].m_vP.isZero() && points[0].m_vC.x() == 456);
	}

	void testTOFsense(void)
	{
		Streams streams;
		_TOFsense sensor;
		streams.configure(sensor, "_TOFsense", {{"nDiv", 3}, {"fovH", 90}, {"vRange", {0.1, 10}}});
		assert(sensor.start());
		streams.send(tofPacket(2, 2000, 321));
		auto points = streams.readAfter(0);
		assert(points.size() == 3);
		assert(points[0].m_tStamp == 0 && points[1].m_tStamp == 0);
		checkPoint(points[2], 2, 321, std::sqrt(3.0f), 1);
		uint64_t previous = points[2].m_tStamp;

		streams.send(tofPacket(0, 4000, 654));
		points = streams.readAfter(previous);
		checkPoint(points[0], 4, 654, 2 * std::sqrt(3.0f), -2);
		assert(points[2].m_tStamp == previous);
		uint64_t stamp = streams.m_pPCLin->getTstamp();
		streams.send(tofPacket(2, 2000, 99, 1));
		points = streams.readAfter(stamp);
		assert(points[2].m_tStamp == 0 && points[2].m_vP.isZero());
		assert(points[2].m_vC.x() == 99);

		stamp = streams.m_pPCLin->getTstamp();
		streams.send(tofPacket(1, -1, 20));
		points = streams.readAfter(stamp);
		assert(points[1].m_tStamp == 0);
		stamp = streams.m_pPCLin->getTstamp();
		streams.send(tofPacket(3, 2000, 10));
		streams.expectUnchanged(stamp);
		auto bytes = tofPacket(0, 2000, 20);
		bytes.back()++;
		streams.send(bytes);
		streams.expectUnchanged(stamp);
	}

	void setRegisters(vector<uint16_t> response)
	{
		std::lock_guard lock(responseMutex);
		registerResponse = std::move(response);
	}

	void setFast(vector<uint8_t> response)
	{
		std::lock_guard lock(responseMutex);
		fastResponse = std::move(response);
	}

	void testLeddarRegisters(void)
	{
		Streams streams;
		LeddarHarness sensor;
		streams.configure(sensor, "_LeddarVu", {{"nDiv", 3}, {"fovH", 90}, {"vRange", {0.1, 10}}});
		sensor.useSimulatedDevice();
		assert(sensor.start());
		vector<uint16_t> response(21, 0);
		response[0] = 1;
		response[1] = 3;
		response[10] = 3;
		response[15] = 200;
		response[16] = 400;
		response[17] = 1000;
		response[18] = 64;
		response[19] = 128;
		response[20] = 192;
		setRegisters(response);
		auto points = streams.readAfter(0);
		assert(points.size() == 3);
		checkPoint(points[0], 2, 1, std::sqrt(3.0f), -1);
		checkPoint(points[1], 4, 2, 4, 0);
		assert(points[0].m_tStamp == points[1].m_tStamp);
		assert(points[2].m_tStamp == 0 && points[2].m_vC.x() == 3);
		uint64_t stamp = streams.m_pPCLin->getTstamp();
		response[1] = 16;
		setRegisters(response);
		streams.expectUnchanged(stamp);
	}

	void testLeddarFast(void)
	{
		Streams streams;
		LeddarHarness sensor;
		streams.configure(sensor, "_LeddarVu", {{"nDiv", 3}, {"fovH", 90}, {"bUse0x41", true}});
		sensor.useSimulatedDevice();
		assert(sensor.start());
		// Two returns from channel 0 and an out-of-bounds channel.
		setFast({1, 0x41, 3, 144, 1, 64, 0, 0, 200, 0, 128, 0, 0, 100, 0, 64, 0, 0xf0});
		auto points = streams.readAfter(0);
		assert(points.size() == 3);
		checkPoint(points[0], 2, 2, std::sqrt(3.0f), -1);
		assert(points[1].m_tStamp == 0 && points[2].m_tStamp == 0);
		uint64_t stamp = streams.m_pPCLin->getTstamp();
		setFast({1, 0x41, 1});
		streams.expectUnchanged(stamp);
		setFast({1, 0x41, 0});
		points = streams.readAfter(stamp);
		assert(points.size() == 3);
		for (const auto &point : points)
		{
			assert(point.m_tStamp == 0);
		}
	}

	void testConfiguration(void)
	{
		Streams streams;
		_TOFsense sensor;
		streams.configure(sensor, "_TOFsense");
		json &j = (*streams.m_cfg.getJson())["sensor"];
		j["PCLframeOut"] = "bytes";
		assert(!sensor.link(&streams.m_instances));
		j.erase("PCLframeOut");
		assert(!sensor.link(&streams.m_instances));
		j["nDiv"] = 0;
		assert(!sensor.loadConfig());
		j["nDiv"] = 257;
		assert(!sensor.loadConfig());
		j["nDiv"] = 1;
		j["fovH"] = 0;
		assert(!sensor.loadConfig());
		j["fovH"] = 90;
		j["vRange"] = {10, 1};
		assert(!sensor.loadConfig());
	}
}

extern "C" int __wrap_modbus_read_input_registers(modbus_t *, int, int count, uint16_t *dest)
{
	std::lock_guard lock(responseMutex);
	if (registerResponse.empty())
	{
		return -1;
	}
	assert(int(registerResponse.size()) <= count);
	int size = registerResponse.size();
	std::copy(registerResponse.begin(), registerResponse.end(), dest);
	registerResponse.clear();
	return size;
}

extern "C" int __wrap_modbus_send_raw_request(modbus_t *, uint8_t *, int count)
{
	return count;
}

extern "C" int __wrap_modbus_receive_confirmation(modbus_t *, uint8_t *dest)
{
	std::lock_guard lock(responseMutex);
	if (fastResponse.empty())
	{
		return -1;
	}
	int size = fastResponse.size();
	std::copy(fastResponse.begin(), fastResponse.end(), dest);
	fastResponse.clear();
	return size;
}

int main()
{
	testBenewake();
	testTOFsense();
	testLeddarRegisters();
	testLeddarFast();
	testConfiguration();
	std::cout << "Distance sensor packet, geometry, validity, and configuration tests passed\n";
}

#endif
