#include "../../src/Actuator/_FTservo.h"
#include "../../src/Instance/Modules.h"
#include <atomic>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <poll.h>
#include <thread>

namespace
{
	using namespace kai;

	class ServoSimulator
	{
	public:
		ServoSimulator()
		{
			m_fd = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
			assert(m_fd >= 0 && grantpt(m_fd) == 0 && unlockpt(m_fd) == 0);
			m_port = ptsname(m_fd);
			m_worker = std::thread(&ServoSimulator::run, this);
		}

		~ServoSimulator()
		{
			disconnect();
		}

		void disconnect()
		{
			m_run = false;
			if (m_worker.joinable())
			{
				m_worker.join();
			}
			if (m_fd >= 0)
			{
				::close(m_fd);
				m_fd = -1;
			}
		}

		std::vector<std::vector<uint8_t>> writes()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_writes;
		}

		string m_port;
		std::atomic<int> m_status{0};
		std::atomic<bool> m_corrupt{false};
		std::atomic<bool> m_silent{false};
		std::atomic<bool> m_fragment{false};
		std::atomic<int> m_position{1234};
		std::atomic<int> m_speed{-1};
		std::atomic<int> m_current{-25};

	private:
		void respond(const std::vector<uint8_t> &packet)
		{
			assert(packet[2] == 7); // All commands must honor the configured ID.
			uint8_t sum = 0;
			for (size_t i = 2; i < packet.size(); ++i)
			{
				sum += packet[i];
			}
			assert(sum == 255);
			std::vector<uint8_t> data;
			if (packet[4] == 3)
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_writes.emplace_back(packet.begin() + 5, packet.end() - 1);
			}
			else
			{
				assert(packet[4] == 2 && packet[5] == 56 && packet[6] == 15);
				data.resize(15);
				putSigned(data, 0, m_position);
				putSigned(data, 2, m_speed);
				data[10] = 1;
				putSigned(data, 13, m_current);
			}
			if (m_silent)
			{
				return;
			}
			std::vector<uint8_t> response = {255, 255, 7, static_cast<uint8_t>(data.size() + 2),
				static_cast<uint8_t>(m_status.load())};
			response.insert(response.end(), data.begin(), data.end());
			sum = 0;
			for (size_t i = 2; i < response.size(); ++i)
			{
				sum += response[i];
			}
			response.push_back(static_cast<uint8_t>(~sum));
			if (m_corrupt.exchange(false))
			{
				response.back() ^= 1;
			}
			if (m_fragment)
			{
				for (uint8_t byte : response)
				{
					assert(::write(m_fd, &byte, 1) == 1);
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
			else
			{
				assert(::write(m_fd, response.data(), response.size()) == static_cast<ssize_t>(response.size()));
			}
		}

		static void putSigned(std::vector<uint8_t> &data, int offset, int value)
		{
			const unsigned encoded = value < 0 ? ((-value) | 0x8000) : value;
			data[offset] = encoded & 255;
			data[offset + 1] = encoded >> 8;
		}

		void run()
		{
			std::vector<uint8_t> pending;
			while (m_run)
			{
				pollfd descriptor = {m_fd, POLLIN, 0};
				if (::poll(&descriptor, 1, 10) <= 0)
				{
					continue;
				}
				uint8_t buffer[256];
				const int count = ::read(m_fd, buffer, sizeof(buffer));
				if (count <= 0)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}
				pending.insert(pending.end(), buffer, buffer + count);
				while (pending.size() >= 4 && pending.size() >= static_cast<size_t>(pending[3] + 4))
				{
					const size_t length = pending[3] + 4;
					respond({pending.begin(), pending.begin() + length});
					pending.erase(pending.begin(), pending.begin() + length);
				}
			}
		}

		int m_fd = -1;
		std::atomic<bool> m_run{true};
		std::thread m_worker;
		std::mutex m_mutex;
		std::vector<std::vector<uint8_t>> m_writes;
	};

	class Driver : public _FTservo
	{
	public:
		using _FTservo::open;
		using _FTservo::close;
		using _FTservo::readStatus;
		using _FTservo::updateMove;

		bool alarm() { return m_bfStatus.b(actuator_alarm); }
		bool originPending() { return m_bfSet.b(actuator_gotoOrigin); }
		bool stopped() { return m_pT->bStopped(); }
	};

	int fdCount()
	{
		return std::distance(std::filesystem::directory_iterator("/proc/self/fd"),
			std::filesystem::directory_iterator());
	}

	void load(Driver &driver, JsonCfg &config)
	{
		driver.setName("FTservo");
		driver.setConfig(&config, &(*config.getJson())["FTservo"]);
		assert(driver.loadConfig());
	}

	void protocolTests(json configJson)
	{
		ServoSimulator simulator;
		configJson["FTservo"]["port"] = simulator.m_port;
		configJson["FTservo"]["ID"] = 7;
		JsonCfg config;
		config.setJson(configJson);
		Driver driver;
		load(driver, config);
		const int descriptorsBefore = fdCount();
		assert(driver.open());
		assert(driver.readStatus());
		assert(driver.pos()->get() == 1234 && driver.speed()->get() == -1 && driver.current()->get() == -25);
		assert(driver.updateMove());
		assert((simulator.writes() == std::vector<std::vector<uint8_t>>{{40, 0}, {33, 0}}));

		driver.power(true);
		assert(driver.updateMove());
		auto writes = simulator.writes();
		assert((writes[writes.size() - 2] == std::vector<uint8_t>{41, 0, 210, 4, 0, 0, 0, 0}));
		assert((writes.back() == std::vector<uint8_t>{40, 1}));
		driver.pos()->setTarget(2345);
		driver.speed()->setTarget(500);
		driver.accel()->setTarget(50);
		driver.move(true);
		assert(driver.updateMove());
		assert((simulator.writes().back() == std::vector<uint8_t>{41, 50, 41, 9, 0, 0, 244, 1}));
		driver.pos()->setTarget(99999);
		assert(driver.updateMove());
		assert((simulator.writes().back() == std::vector<uint8_t>{41, 50, 255, 15, 0, 0, 244, 1}));
		driver.move(false);
		simulator.m_position = 1300;
		assert(driver.updateMove());
		assert((simulator.writes().back() == std::vector<uint8_t>{41, 0, 20, 5, 0, 0, 0, 0}));

		driver.gotoOrigin();
		assert(driver.updateMove() && !driver.originPending());
		driver.move(false);
		assert(driver.updateMove());
		assert((simulator.writes().back() == std::vector<uint8_t>{41, 0, 20, 5, 0, 0, 0, 0}));
		driver.gotoOrigin();
		assert(driver.updateMove());
		simulator.m_position = 2048;
		assert(driver.readStatus() && driver.bComplete());
		driver.setMode(1);
		driver.speed()->setTarget(-600);
		driver.move(true);
		assert(driver.updateMove());
		assert((simulator.writes().back() == std::vector<uint8_t>{41, 50, 0, 0, 0, 0, 88, 130}));
		driver.speed()->setTarget(-99999);
		driver.accel()->setTarget(300);
		assert(driver.updateMove());
		assert((simulator.writes().back() == std::vector<uint8_t>{41, 255, 0, 0, 0, 0, 232, 131}));
		driver.move(false);
		assert(driver.updateMove());
		assert((simulator.writes().back() == std::vector<uint8_t>{41, 0, 0, 0, 0, 0, 0, 0}));
		driver.power(false);
		assert(driver.updateMove());
		assert((simulator.writes().back() == std::vector<uint8_t>{40, 0}));
		const size_t nWrites = simulator.writes().size();
		driver.move(true);
		assert(driver.updateMove() && simulator.writes().size() == nWrites);

		// Failed feedback must not publish stale/corrupted values.
		simulator.m_position = 999;
		simulator.m_corrupt = true;
		assert(!driver.readStatus() && driver.alarm() && driver.pos()->get() == 2048);
		simulator.m_status = 4;
		assert(!driver.readStatus() && driver.alarm() && driver.pos()->get() == 2048);
		simulator.m_status = 0;
		simulator.m_fragment = true;
		assert(driver.readStatus() && !driver.alarm() && driver.pos()->get() == 999);
		simulator.m_fragment = false;
		simulator.m_silent = true;
		assert(!driver.readStatus());
		simulator.m_silent = false;
		driver.close();
		assert(fdCount() == descriptorsBefore);
		for (int i = 0; i < 5; ++i)
		{
			assert(driver.open() && driver.updateMove());
			driver.close();
			assert(fdCount() == descriptorsBefore);
		}
		assert(driver.saveConfig(false));
		assert((*config.getJson())["FTservo"]["ID"] == 7);
		assert((*config.getJson())["FTservo"]["port"] == simulator.m_port);

		// Real worker retries after an error and reapplies mode on reconnection.
		simulator.m_status = 4;
		assert(driver.start());
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
		const size_t writesBeforeRecovery = simulator.writes().size();
		simulator.m_status = 0;
		std::this_thread::sleep_for(std::chrono::milliseconds(1200));
		assert(simulator.writes().size() > writesBeforeRecovery);
		assert((simulator.writes().back() == std::vector<uint8_t>{33, 1}));
		driver.stop();
		assert(driver.stopped() && fdCount() == descriptorsBefore);

		// Worker startup, stop, and restart, followed by a cable disconnect.
		for (int i = 0; i < 2; ++i)
		{
			assert(driver.start());
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			driver.stop();
			assert(driver.stopped() && fdCount() == descriptorsBefore);
		}
		assert(driver.start());
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
		simulator.disconnect();
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
		driver.stop();
		assert(driver.stopped());
		std::cout << "PASS: protocol, motion, feedback errors, reconnect cleanup, and worker shutdown\n";
	}

	void configTests(const json &configJson)
	{
		for (const json &invalid : {json{{"ID", 254}}, json{{"ID", -1}}, json{{"mode", 3}},
			json{{"baud", 12345}}, json{{"port", ""}}, json{{"pRange", {4, 1}}}, json{{"bfSetID", true}}})
		{
			json modified = configJson;
			modified["FTservo"].update(invalid);
			JsonCfg config;
			config.setJson(modified);
			Driver driver;
			driver.setName("FTservo");
			driver.setConfig(&config, &(*config.getJson())["FTservo"]);
			assert(!driver.loadConfig());
		}
		json modified = configJson;
		modified["FTservo"]["port"] = "/tmp/openkai-ftservo-nonexistent-device";
		JsonCfg config;
		config.setJson(modified);
		Driver driver;
		load(driver, config);
		driver.stop();
		for (int i = 0; i < 3; ++i)
		{
			assert(driver.start());
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
			driver.stop();
			assert(driver.stopped());
		}
		// begin() can open a path then fail termios setup; that fd must close too.
		(*config.getJson())["FTservo"]["port"] = "/dev/null";
		assert(driver.loadConfig());
		const int descriptorsBefore = fdCount();
		assert(!driver.open() && fdCount() == descriptorsBefore);
		Modules modules;
		std::unique_ptr<_ModuleBase> instance(modules.createInstance("_FTservo"));
		assert(dynamic_cast<_FTservo *>(instance.get()));
		std::cout << "PASS: config validation, factory registration, and disconnected lifecycle\n";
	}
}

int main(int argc, char **argv)
{
	assert(argc == 2);
	std::ifstream input(argv[1]);
	json config;
	input >> config;
	configTests(config);
	protocolTests(config);
}
