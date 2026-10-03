#include "../../src/UI/Viewer/Web/WebMavlinkProtocol.h"
#include "../../src/UI/Viewer/Web/_WebMavlinkStream.h"
#include "../../src/IO/WebSocketStream.h"
#undef timeout
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <thread>

namespace
{
	using namespace kai;
	class Stream : public MavlinkStream
	{
	public:
		size_t pending() const { return m_vMsgQueue.size(); }
	};
	bool near(const json &value, double expected) { return std::abs(value.get<double>() - expected) < 1e-6; }
	json frame(Stream &stream, uint64_t now = getTns()) { return webmavlink::telemetry(stream, 7, now, 1234, 3000); }

	void telemetryTests()
	{
		Stream stream;
		const auto initial = frame(stream);
		assert(initial["protocol"] == "openkai.mavlink" && initial["version"] == 1);
		assert(!initial["connected"].get<bool>() && initial["position"].is_null() && initial["attitude"].is_null());
		assert(initial["battery"].is_null() && initial["heartbeat"].is_null());
		assert(initial["timeMs"] == 1234 && initial["sequence"] == 7);
		const auto hello = webmavlink::hello({{"drone", {{"url", "/models/drone/multirotor.glb"}}}}, 3000);
		assert(hello["readOnly"] == true && hello["config"]["staleAfterMs"] == 3000);

		mavlink_message_t encoded{};
		mavlink_msg_heartbeat_pack(1, 1, &encoded, MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_ARDUPILOTMEGA,
			MAV_MODE_FLAG_SAFETY_ARMED, 9, MAV_STATE_ACTIVE);
		assert(stream.decode(encoded));
		mavlink_global_position_int_t position{};
		position.lat = 356812000; position.lon = 1397671000; position.alt = 51250; position.relative_alt = 20500;
		position.vx = 300; position.vy = 400; position.vz = -150; position.hdg = UINT16_MAX;
		mavlink_msg_global_position_int_encode(1, 1, &encoded, &position);
		assert(stream.decode(encoded));
		mavlink_attitude_t attitude{};
		attitude.roll = 0.1f; attitude.pitch = -0.2f; attitude.yaw = 0.3f;
		mavlink_msg_attitude_encode(1, 1, &encoded, &attitude);
		assert(stream.decode(encoded));
		mavlink_sys_status_t system{};
		system.voltage_battery = 24500; system.current_battery = -1; system.battery_remaining = -1; system.load = 432;
		mavlink_msg_sys_status_encode(1, 1, &encoded, &system);
		assert(stream.decode(encoded));
		mavlink_gps_raw_int_t gps{};
		gps.fix_type = 3; gps.lat = position.lat; gps.lon = position.lon; gps.alt = position.alt;
		gps.eph = UINT16_MAX; gps.epv = 120; gps.satellites_visible = UINT8_MAX;
		mavlink_msg_gps_raw_int_encode(1, 1, &encoded, &gps);
		assert(stream.decode(encoded));
		auto j = frame(stream);
		assert(j["connected"] == true && j["heartbeat"]["armed"] == true && j["heartbeat"]["customMode"] == 9);
		assert(near(j["position"]["latitudeDeg"], 35.6812) && near(j["position"]["longitudeDeg"], 139.7671));
		assert(near(j["position"]["altitudeMslM"], 51.25) && near(j["position"]["relativeAltitudeM"], 20.5));
		assert(near(j["position"]["groundSpeedMps"], 5) && near(j["position"]["velocityNedMps"][2], -1.5));
		assert(j["position"]["headingDeg"].is_null());
		assert(near(j["attitude"]["pitchRad"], -0.2) && j["attitude"]["valid"] == true);
		assert(j["battery"]["source"] == "SYS_STATUS" && near(j["battery"]["voltageV"], 24.5));
		assert(j["battery"]["currentA"].is_null() && j["battery"]["remainingPct"].is_null());
		assert(near(j["system"]["loadPct"], 43.2));
		assert(j["gps"]["hdop"].is_null() && j["gps"]["satellitesVisible"].is_null() && j["gps"]["altitudeEllipsoidM"].is_null());
		j = frame(stream, getTns() + 4000000000ULL);
		assert(j["connected"] == false && j["position"]["stale"] == true && j["attitude"]["stale"] == true);

		mavlink_attitude_quaternion_t quaternion{};
		quaternion.q1 = std::sqrt(2.f); quaternion.q4 = std::sqrt(2.f);
		mavlink_msg_attitude_quaternion_encode(1, 1, &encoded, &quaternion);
		assert(stream.decode(encoded));
		j = frame(stream);
		assert(j["attitude"]["source"] == "ATTITUDE_QUATERNION" && near(j["attitude"]["yawRad"], M_PI / 2));
		assert(near(j["attitude"]["quaternionWxyz"][0], std::sqrt(0.5)));
		quaternion.q1 = NAN;
		mavlink_msg_attitude_quaternion_encode(1, 1, &encoded, &quaternion);
		assert(stream.decode(encoded));
		assert(frame(stream)["attitude"]["source"] == "ATTITUDE");

		mavlink_battery_status_t battery{};
		for (size_t i = 0; i < 10; ++i) battery.voltages[i] = UINT16_MAX;
		battery.voltages[0] = 4000; battery.voltages[1] = 4100; battery.voltages_ext[0] = 3900;
		battery.current_battery = 125; battery.temperature = INT16_MAX; battery.current_consumed = -1; battery.battery_remaining = 80;
		mavlink_msg_battery_status_encode(1, 1, &encoded, &battery);
		assert(stream.decode(encoded));
		j = frame(stream);
		assert(j["battery"]["source"] == "BATTERY_STATUS" && near(j["battery"]["voltageV"], 12));
		assert(near(j["battery"]["currentA"], 1.25) && j["battery"]["remainingPct"] == 80);
		assert(j["battery"]["temperatureC"].is_null() && j["battery"]["consumedMah"].is_null());

		mavlink_home_position_t home{};
		home.latitude = position.lat; home.longitude = position.lon; home.altitude = 10000;
		mavlink_msg_home_position_encode(1, 1, &encoded, &home);
		assert(stream.decode(encoded));
		assert(near(frame(stream)["home"]["altitudeMslM"], 10));
		mavlink_statustext_t status{};
		std::memset(status.text, 'A', sizeof(status.text));
		status.text[1] = char(0xff); status.severity = 4; status.id = 12; status.chunk_seq = 1;
		mavlink_msg_statustext_encode(1, 1, &encoded, &status);
		assert(stream.decode(encoded));
		j = frame(stream);
		assert(j["statusText"]["text"].get<std::string>().size() == 50 && j["statusText"]["chunkSeq"] == 1);
		const auto safe = j.dump(-1, ' ', false, json::error_handler_t::replace);
		assert(json::parse(safe)["statusText"]["text"].get<std::string>().find("\xef\xbf\xbd") != std::string::npos);
		assert(stream.pending() == 0); // The viewer never enqueues outbound MAVLink messages.
	}

	void concurrentSnapshotTest()
	{
		Stream stream;
		std::atomic<bool> complete{false};
		std::thread writer([&] {
			for (int i = 1; i <= 10000; ++i)
			{
				mavlink_global_position_int_t position{};
				position.lat = i; position.lon = -i; position.alt = i * 10;
				mavlink_message_t encoded{};
				mavlink_msg_global_position_int_encode(1, 1, &encoded, &position);
				assert(stream.decode(encoded));
			}
			complete = true;
		});
		while (!complete)
		{
			mavlink_global_position_int_t position{};
			uint64_t timestamp = 0;
			if (!stream.snapshot<MavGlobalPositionINT>(position, timestamp)) continue;
			assert(timestamp > 0 && position.lon == -position.lat && position.alt == position.lat * 10);
		}
		writer.join();
	}

	void moduleConfigTest(bool network)
	{
		JsonCfg cfg;
		cfg.setJson({{"vehicle", {{"class", "MavlinkStream"}}}, {"viewer", {{"class", "_WebMavlinkStream"},
			{"MavlinkStream", "vehicle"}, {"webRoot", "html/viewer/mavlink"}, {"modelsRoot", "/missing/test-models"},
			{"thread", {{"FPS", 20}}}, {"scene", {{"trailMaxPoints", 500}}}}}});
		InstanceMgr instances;
		json &config = *cfg.getJson();
		assert(instances.addDataObject("vehicle", &cfg, &config["vehicle"]));
		assert(instances.initAll());
		_WebMavlinkStream viewer;
		viewer.setName("viewer"); viewer.setConfig(&cfg, &config["viewer"]);
		assert(viewer.loadConfig() && viewer.link(&instances) && viewer.check());
		assert(viewer.saveConfig(false));
		assert(config["viewer"]["modelsRoot"] == "/missing/test-models" && config["viewer"]["MavlinkStream"] == "vehicle");
		config["viewer"]["port"] = -1;
		assert(!viewer.loadConfig());
		if (!network) return;
		namespace net = boost::asio;
		namespace beast = boost::beast;
		net::io_context io;
		net::ip::tcp::acceptor portReservation(io, {net::ip::make_address("127.0.0.1"), 0});
		const auto port = portReservation.local_endpoint().port();
		portReservation.close();
		config["viewer"]["port"] = port;
		config["viewer"]["host"] = "127.0.0.1";
		assert(viewer.loadConfig() && viewer.link(&instances));
		for (int attempt = 0; attempt < 2; ++attempt)
		{
			assert(viewer.start());
			beast::websocket::stream<net::ip::tcp::socket> client(io);
			client.next_layer().connect({net::ip::make_address("127.0.0.1"), port});
			client.handshake("localhost", "/stream/mavlink");
			beast::flat_buffer buffer;
			client.read(buffer);
			assert(json::parse(beast::buffers_to_string(buffer.data()))["type"] == "hello");
			buffer.consume(buffer.size());
			client.read(buffer);
			assert(json::parse(beast::buffers_to_string(buffer.data()))["type"] == "telemetry");
			client.close(beast::websocket::close_code::normal);
			viewer.stop();
			assert(viewer.bStopped());
		}
	}

	void websocketTest(WebSocketStream::Mode mode)
	{
		namespace net = boost::asio;
		namespace beast = boost::beast;
		namespace ws = beast::websocket;
		HttpServer server;
		WebSocketStream transport(server.context(), "hello", 8, mode);
		std::string error;
		assert(server.start("127.0.0.1", 0, ".", WebSocketStream::routes({{"/stream/mavlink", &transport}}), &error));
		auto publish = [&](char value) { transport.publish(std::make_shared<std::vector<uint8_t>>(1, uint8_t(value))); };
		publish('1');
		net::io_context io;
		ws::stream<net::ip::tcp::socket> client(io);
		client.next_layer().connect({net::ip::make_address("127.0.0.1"), server.port()});
		client.handshake("localhost", "/stream/mavlink");
		auto read = [&] { beast::flat_buffer buffer; client.read(buffer); return beast::buffers_to_string(buffer.data()); };
		assert(read() == "hello" && client.got_text());
		if (mode == WebSocketStream::Mode::BinaryAcknowledged)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			assert(client.next_layer().available() == 0);
			client.write(net::buffer(std::string("start")));
		}
		assert(read() == "1" && client.got_text() == (mode == WebSocketStream::Mode::TextPush));
		publish('2');
		if (mode == WebSocketStream::Mode::BinaryAcknowledged)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
			assert(client.next_layer().available() == 0);
			client.write(net::buffer(std::string("next")));
		}
		assert(read() == "2");
		if (mode == WebSocketStream::Mode::TextPush)
		{
			client.write(net::buffer(std::string("start"))); // Geometry commands are not accepted here.
			beast::flat_buffer buffer;
			beast::error_code ec;
			client.read(buffer, ec);
			assert(ec);
		}
		else client.close(ws::close_code::normal);
		server.stop(); transport.stop();
	}
}

int main(int argc, char **argv)
{
	telemetryTests();
	concurrentSnapshotTest();
	const bool network = argc < 2 || std::string(argv[1]) != "--no-network";
	moduleConfigTest(network);
	if (network)
	{
		websocketTest(kai::WebSocketStream::Mode::BinaryAcknowledged);
		websocketTest(kai::WebSocketStream::Mode::TextPush);
	}
	std::cout << "MAVLink viewer telemetry, concurrent snapshots, config and selected transport tests passed\n";
}
