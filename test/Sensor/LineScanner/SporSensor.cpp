#include "../../../src/Sensor/LineScanner/_SporSensor.h"
#include "../../../src/Protocol/_JSONbase.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>

namespace
{
	using namespace kai;

	class SporHarness : public _SporSensor
	{
	public:
		using _SporSensor::applyConfig;
		using _SporSensor::configValues;
		using _SporSensor::controlSchema;
		using _SporSensor::publishFrame;

		void output(PCLframe *frame) { m_pPCLout = frame; }

		void configure(const json &values)
		{
			json errors;
			if (!applyConfig(values, false, errors))
			{
				std::cerr << "Configuration rejected: " << values << "\n" << errors << "\n";
				assert(false);
			}
		}
	};

	class ReplyTransport : public _JSONbase
	{
	public:
		vector<json> replies;
		bool sendJson(const json &reply) override
		{
			replies.push_back(reply);
			return true;
		}
	};

	void near(float actual, double expected)
	{
		assert(std::abs(actual - expected) < 1e-6);
	}

	vector<GEOMETRY_POINT> read(PCLframe &output, uint64_t before, uint64_t after)
	{
		vector<GEOMETRY_POINT> points;
		const auto stamp = output.get(points);
		assert(stamp >= before && stamp <= after);
		for (const auto &point : points)
		{
			assert(point.m_tStamp == stamp);
			assert(point.m_vP.allFinite());
		}
		return points;
	}

	spor::ProfileFrame synthetic()
	{
		spor::ProfileFrame frame;
		frame.frame_id = 42;
		// Deliberately unrelated to the host clock. The SDK does not define the
		// timestamp's unit, so it must never become a GEOMETRY_POINT timestamp.
		frame.timestamp = std::numeric_limits<uint64_t>::max();
		frame.encoder = 0x123456789abcdef0ULL;
		spor::ProfileLine top;
		top.points = {
			{1000, 250, true}, {-500, -125, true}, {42, 12, false},
			{std::numeric_limits<double>::quiet_NaN(), 1, true},
			{1, std::numeric_limits<double>::infinity(), true},
			{spor::invalid_coordinate, 1, true},
			{1, spor::invalid_coordinate, true}, {0, 0, true},
		};
		top.has_brightness = true;
		top.brightness = {0, 255, 12, 13, 14, 15, 16, 128};
		frame.lines.push_back(top);
		spor::ProfileLine bottom;
		bottom.side = spor::ProfileSide::bottom;
		bottom.points = {{20, 30, true}};
		frame.lines.push_back(bottom);
		return frame;
	}

	void testPointConversion()
	{
		PCLframe output;
		SporHarness sensor;
		sensor.output(&output);
		sensor.configure({{"fallbackColor", {0.2, 0.4, 0.6}}, {"yOffsetM", 0.125}});
		const auto frame = synthetic();
		const auto before = getTns();
		sensor.publishFrame(frame);
		const auto points = read(output, before, getTns());
		assert(points.size() == 4);
		near(points[0].m_vP.x(), 1.0);
		near(points[0].m_vP.y(), 0.125);
		near(points[0].m_vP.z(), 0.25);
		assert(points[0].m_vC.isZero()); // Present zero intensity is real black.
		near(points[1].m_vP.x(), -0.5);
		near(points[1].m_vP.z(), -0.125);
		assert(points[1].m_vC == Vector3f::Ones());
		near(points[2].m_vC.x(), 128.0 / 255.0);
		assert(points[2].m_vC.x() == points[2].m_vC.y());
		assert(points[2].m_vC.x() == points[2].m_vC.z());
		near(points[3].m_vP.x(), 0.02);
		near(points[3].m_vP.z(), 0.03);
		near(points[3].m_vC.x(), 0.2);
		near(points[3].m_vC.y(), 0.4);
		near(points[3].m_vC.z(), 0.6);

		sensor.configure({{"profileSide", "bottom"}, {"colorMode", "solid"}});
		sensor.publishFrame(frame);
		vector<GEOMETRY_POINT> selected;
		output.get(selected);
		assert(selected.size() == 1);
		near(selected[0].m_vP.x(), 0.02);
		near(selected[0].m_vC.y(), 0.4);

		sensor.configure({{"profileSide", "top"}, {"pointStride", 2}});
		sensor.publishFrame(frame);
		output.get(selected);
		assert(selected.size() == 1); // Sample-index stride, before invalid filtering.
		near(selected[0].m_vP.x(), 1.0);

		// An empty frame replaces the previous cloud instead of displaying stale
		// measurements when all sensor samples become invalid.
		spor::ProfileFrame empty;
		sensor.publishFrame(empty);
		output.get(selected);
		assert(selected.empty());
	}

	void testConfigurationValidation()
	{
		SporHarness sensor;
		sensor.configure({
			{"address", "192.168.80.8"}, {"sensorId", "10000845"},
			{"controlPort", 11061}, {"dataPort", 11062}, {"notificationPort", 11063},
			{"publishPort", 11059}, {"timeoutMs", 5000}, {"uiTimeoutMs", 9000},
			{"reconnectMs", 1500}, {"pointStride", 2}, {"yOffsetM", -0.1},
			{"allowUnverifiedCompression", false}, {"autoStart", false},
			{"laserPower", 80}, {"exposureUs", 500}, {"brightnessEnabled", true},
			{"frameRateHz", 100}, {"scanRateHz", nullptr}, {"laserTimingMode", 0},
			{"triggerMode", 0}, {"externalTrigger", false},
			{"areaXRangeMm", 100}, {"areaZRangeMm", 100},
			{"areaXStartMm", -50}, {"areaZStartMm", -50},
			{"transformXmm", 1}, {"transformYmm", 2}, {"transformZmm", 3},
			{"transformXdeg", 4}, {"transformYdeg", 5}, {"transformZdeg", 6},
			{"filterGapXEnabled", false}, {"filterGapXAmount", 1},
		});
		const auto previous = sensor.configValues();
		const vector<json> invalid = {
			{{"laserPower", -1}}, {{"laserPower", 101}}, {{"laserPower", "80"}},
			{{"exposureUs", 0}}, {{"exposureUs", -1}}, {{"exposureUs", 1.5}},
			{{"exposureUs", uint64_t(UINT32_MAX) + 1}},
			{{"pointStride", 0}}, {{"pointStride", 1.5}},
			{{"profileSide", "left"}}, {{"colorMode", "unknown"}},
			{{"fallbackColor", {0.1, 0.2}}}, {{"fallbackColor", {1.1, 0, 0}}},
			{{"controlPort", 0}}, {{"dataPort", 65536}},
			{{"timeoutMs", 0}}, {{"maxPointsPerMessage", 0}},
			{{"laserState", 3}}, {{"triggerMode", 4}}, {{"laserTimingMode", 2}},
			{{"brightnessEnabled", 1}}, {{"areaXRangeMm", -1}},
			{{"frameRateHz", 0}}, {{"yOffsetM", std::numeric_limits<double>::infinity()}},
			{{"pointStride", 4}, {"laserPower", -1}},
			{{"pointStride", 4}, {"unknownOption", 1}},
			{{"frameRateHz", 100}, {"scanRateHz", 100}},
			{{"areaXRangeMm", nullptr}},
		};
		for (const auto &values : invalid)
		{
			json errors;
			if (sensor.applyConfig(values, false, errors))
			{
				std::cerr << "Invalid settings accepted: " << values << "\n";
				assert(false);
			}
			assert(!errors.empty());
			assert(sensor.configValues() == previous); // Whole request rejected.
		}
		sensor.configure({{"laserPower", nullptr}, {"exposureUs", nullptr},
			{"brightnessEnabled", nullptr}, {"filterGapXEnabled", nullptr},
			{"filterGapXAmount", nullptr}});
		assert(sensor.configValues()["laserPower"].is_null());
		assert(sensor.configValues()["exposureUs"].is_null());
	}

	void testConfigurationRoundTrip()
	{
		JsonCfg config;
		config.setJson({{"scanner", {{"class", "_SporSensor"},
			{"PCLframeOut", "points"}, {"thread", {{"FPS", 0}}}}}});
		json &stored = (*config.getJson())["scanner"];
		SporHarness sensor;
		sensor.setConfig(&config, &stored);
		sensor.setName("scanner");
		assert(sensor.loadConfig());
		const auto defaults = sensor.configValues();
		assert(defaults.is_object() && defaults.size() >= 60);
		assert(sensor.saveConfig(false));
		for (auto it = defaults.begin(); it != defaults.end(); ++it)
		{
			assert(stored.contains(it.key()));
			assert(stored[it.key()] == it.value());
		}
		json changed = {
			{"address", "192.168.80.9"}, {"localAddress", "192.168.80.20"},
			{"sensorId", "test_sensor"}, {"controlPort", 12061},
			{"notificationPort", 12063}, {"dataPort", 12062}, {"publishPort", 12059},
			{"protocolVersion", "2.6.0.3"}, {"timeoutMs", 6000}, {"uiTimeoutMs", 10000},
			{"streamTimeoutMs", 12000},
			{"reconnectMs", 2000}, {"maxNotificationBytes", 2097152},
			{"maxMessageBytes", 33554432}, {"maxFramesPerMessage", 2048},
			{"maxLinesPerGroup", 32}, {"maxPointsPerLine", 524288},
			{"maxPointsPerMessage", 2097152}, {"allowUnverifiedCompression", true},
			{"ensureSdkServerEnabled", false}, {"useDiscoveredPorts", false},
			{"autoStart", false}, {"profileSide", "top"}, {"pointStride", 3},
			{"colorMode", "solid"}, {"fallbackColor", {0.1, 0.3, 0.7}},
			{"yOffsetM", 0.2}, {"scanMode", 2}, {"scanRateHz", 50},
			{"exposureUs", 750}, {"laserState", 2}, {"triggerMode", 3},
			{"externalTrigger", true}, {"laserTimingMode", 1}, {"laserPower", 37.5},
			{"brightnessEnabled", false}, {"frameRateHz", nullptr},
			{"measurementOutputEnabled", false},
			{"areaXRangeMm", 80}, {"areaZRangeMm", 90},
			{"areaXStartMm", -40}, {"areaZStartMm", -45},
			{"transformXmm", 1}, {"transformYmm", 2}, {"transformZmm", 3},
			{"transformXdeg", 4}, {"transformYdeg", 5}, {"transformZdeg", 6},
		};
		for (const string kind : {"Gap", "Median", "Smoothing", "Decimation"})
		{
			for (const string axis : {"X", "Y"})
			{
				changed["filter" + kind + axis + "Enabled"] = (axis == "X");
				changed["filter" + kind + axis + "Amount"] = (axis == "X") ? 1.5 : 2.5;
			}
		}
		// Every declared option must survive a nondefault load/save cycle.
		assert(changed.size() == defaults.size());
		sensor.configure(changed);
		assert(sensor.saveConfig(false));
		SporHarness restored;
		restored.setConfig(&config, &stored);
		restored.setName("restored");
		assert(restored.loadConfig());
		assert(restored.configValues() == sensor.configValues());
		assert(restored.controlSchema() == sensor.controlSchema());

		json preserve = json::object();
		for (auto it = defaults.begin(); it != defaults.end(); ++it)
			if (it.value().is_null())
				preserve[it.key()] = nullptr;
		restored.configure(preserve);
		assert(restored.saveConfig(false));
		for (auto it = preserve.begin(); it != preserve.end(); ++it)
			assert(stored[it.key()].is_null());
	}

	void testCapturedProfiles(const string &directory)
	{
		auto decode = [&](const string &filename) {
			std::ifstream input(directory + "/" + filename, std::ios::binary);
			assert(input);
			vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
			return spor::ProfileDecoder({}, "2.6.0.2").decode(bytes);
		};
		PCLframe output;
		SporHarness sensor;
		sensor.output(&output);
		const auto frames = decode("profile-brightness-v2.6.0.2.bin");
		assert(frames.size() == 1 && frames[0].lines[0].points.size() == 4286);
		sensor.publishFrame(frames[0]);
		vector<GEOMETRY_POINT> points;
		output.get(points);
		assert(points.size() == 7);
		// These expectations come from independently inspected capture bytes,
		// not the adapter's output or another conversion of the decoded points.
		const double xs[] = {-64.418, -50.648, -47.894, -14.744, 23.71, 23.812, 35.44};
		const double zs[] = {242.9333333333, 243.05, 243.075, 242.8083333333,
			241.825, 241.8166666667, 241.3166666667};
		const unsigned brightness[] = {18, 19, 19, 19, 20, 22, 20};
		for (size_t i = 0; i < points.size(); ++i)
		{
			near(points[i].m_vP.x(), xs[i] * 0.001);
			near(points[i].m_vP.y(), 0);
			near(points[i].m_vP.z(), zs[i] * 0.001);
			near(points[i].m_vC.x(), brightness[i] / 255.0);
			assert(points[i].m_tStamp != frames[0].timestamp);
		}
		const auto invalid = decode("profile-invalid-v2.6.0.2.bin");
		assert(invalid.size() == 1);
		sensor.publishFrame(invalid[0]);
		output.get(points);
		assert(points.empty());
	}

	void testConsoleProtocol()
	{
		SporHarness sensor;
		sensor.setName("scanner");
		ReplyTransport transport;
		auto command = [&](const json &request) {
			const auto count = transport.replies.size();
			sensor.console(request, &transport);
			assert(transport.replies.size() == count + 1);
			const auto &reply = transport.replies.back();
			assert(reply["cmd"] == request["cmd"]);
			assert(reply["module"] == "scanner");
			if (request.contains("requestId"))
				assert(reply["requestId"] == request["requestId"]);
			return reply;
		};
		auto reply = command({{"cmd", "getConfig"}, {"requestId", "request-1"}});
		assert(reply["bSuccess"] == true);
		assert(reply["schema"].size() == reply["config"].size());
		assert(reply["actions"].is_array() && reply["actions"].size() >= 6);
		assert(reply["deviceOpen"] == false && reply["status"]["connected"] == false);

		const auto previous = reply["config"];
		reply = command({{"cmd", "setConfig"}, {"requestId", 2},
			{"config", {{"pointStride", 4}, {"laserPower", -1}}}});
		assert(reply["bSuccess"] == false && reply["errors"].contains("laserPower"));
		assert(reply["config"] == previous);
		reply = command({{"cmd", "setConfig"}, {"config", json::array()}});
		assert(reply["bSuccess"] == false && reply["errors"].contains("config"));
		reply = command({{"cmd", "setConfig"}, {"config", {{"pointStride", 3}}}});
		assert(reply["bSuccess"] == true && reply["pending"] == true);
		assert(reply["config"]["pointStride"] == 3);

		for (const string action : {"startAcquisition", "stopAcquisition", "softwareTrigger",
			"resetSensorArea", "setMaximumFrameRate", "readProject", "readModel"})
		{
			reply = command({{"cmd", action}});
			assert(reply["bSuccess"] == false && reply.contains("error"));
			assert(reply["deviceOpen"] == false);
		}
		const auto count = transport.replies.size();
		sensor.console({{"cmd", "unsupportedAction"}}, &transport);
		sensor.console({{"cmd", 1}}, &transport);
		assert(transport.replies.size() == count);
	}
}

int main(int argc, char **argv)
{
	assert(argc == 2);
	testConfigurationValidation();
	testConfigurationRoundTrip();
	testPointConversion();
	testCapturedProfiles(argv[1]);
	testConsoleProtocol();
	std::cout << "SporSensor configuration, profile geometry, timestamp, intensity, "
		"side selection, stride, real capture, and console protocol regressions passed\n";
}
