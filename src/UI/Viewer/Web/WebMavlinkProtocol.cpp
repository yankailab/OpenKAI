#include "WebMavlinkProtocol.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace kai::webmavlink
{
	namespace
	{
		json envelope(const char *type)
		{
			return {{"protocol", Protocol}, {"version", Version}, {"type", type}};
		}
		json age(uint64_t timestamp, uint64_t now, uint64_t staleAfterMs)
		{
			const uint64_t ms = now > timestamp ? (now - timestamp) / 1000000 : 0;
			return {{"ageMs", ms}, {"stale", ms >= staleAfterMs}};
		}
		json finite(double value) { return std::isfinite(value) ? json(value) : json(nullptr); }
		json percent(int value) { return value >= 0 && value <= 100 ? json(value) : json(nullptr); }
		bool coordinates(int32_t lat, int32_t lon)
		{
			return lat >= -900000000 && lat <= 900000000 && lon >= -1800000000 && lon <= 1800000000;
		}
	}

	json hello(const json &scene, uint64_t staleAfterMs)
	{
		json result = envelope("hello");
		result["readOnly"] = true;
		result["config"] = scene;
		result["config"]["staleAfterMs"] = staleAfterMs;
		return result;
	}

	json telemetry(MavlinkStream &stream, uint64_t sequence, uint64_t nowNs,
		uint64_t unixTimeMs, uint64_t staleAfterMs)
	{
		json result = envelope("telemetry");
		result["sequence"] = sequence;
		result["timeMs"] = unixTimeMs;
		result["connected"] = false;
		for (const char *field : {"heartbeat", "position", "attitude", "battery", "gps", "system", "home", "statusText"})
			result[field] = nullptr;

		uint64_t timestamp = 0;
		mavlink_heartbeat_t heartbeat{};
		if (stream.snapshot<MavHeartbeat>(heartbeat, timestamp))
		{
			json j = age(timestamp, nowNs, staleAfterMs);
			j["armed"] = (heartbeat.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) != 0;
			j["baseMode"] = heartbeat.base_mode;
			j["customMode"] = heartbeat.custom_mode;
			j["systemStatus"] = heartbeat.system_status;
			j["vehicleType"] = heartbeat.type;
			j["autopilot"] = heartbeat.autopilot;
			result["connected"] = !j["stale"].get<bool>();
			result["heartbeat"] = std::move(j);
		}

		mavlink_global_position_int_t position{};
		if (stream.snapshot<MavGlobalPositionINT>(position, timestamp))
		{
			json j = age(timestamp, nowNs, staleAfterMs);
			j["valid"] = coordinates(position.lat, position.lon);
			j["latitudeDeg"] = position.lat * 1e-7;
			j["longitudeDeg"] = position.lon * 1e-7;
			j["altitudeMslM"] = position.alt * 0.001;
			j["relativeAltitudeM"] = position.relative_alt * 0.001;
			j["velocityNedMps"] = {position.vx * 0.01, position.vy * 0.01, position.vz * 0.01};
			j["groundSpeedMps"] = std::hypot(double(position.vx), double(position.vy)) * 0.01;
			j["headingDeg"] = position.hdg <= 35999 ? json(position.hdg * 0.01) : json(nullptr);
			result["position"] = std::move(j);
		}

		mavlink_attitude_t euler{};
		mavlink_attitude_quaternion_t quaternion{};
		uint64_t eulerTime = 0, quaternionTime = 0;
		const bool hasEuler = stream.snapshot<MavAttitude>(euler, eulerTime);
		const bool hasQuaternion = stream.snapshot<MavAttitudeQuaternion>(quaternion, quaternionTime);
		const double norm = std::sqrt(double(quaternion.q1) * quaternion.q1 + double(quaternion.q2) * quaternion.q2 +
			double(quaternion.q3) * quaternion.q3 + double(quaternion.q4) * quaternion.q4);
		const bool validQuaternion = hasQuaternion && std::isfinite(norm) && norm > 1e-9;
		const bool validEuler = hasEuler && std::isfinite(euler.roll) && std::isfinite(euler.pitch) && std::isfinite(euler.yaw);
		if (hasEuler || hasQuaternion)
		{
			// Prefer the newest usable attitude; bad quaternions do not hide valid Euler data.
			const bool useQuaternion = validQuaternion && (!validEuler || quaternionTime >= eulerTime);
			const bool valid = useQuaternion || validEuler;
			json j = age(useQuaternion ? quaternionTime : (hasEuler ? eulerTime : quaternionTime), nowNs, staleAfterMs);
			j["valid"] = valid;
			j["source"] = useQuaternion ? "ATTITUDE_QUATERNION" : "ATTITUDE";
			double w = 1, x = 0, y = 0, z = 0, roll = 0, pitch = 0, yaw = 0;
			if (useQuaternion)
			{
				w = quaternion.q1 / norm; x = quaternion.q2 / norm;
				y = quaternion.q3 / norm; z = quaternion.q4 / norm;
				roll = std::atan2(2 * (w*x + y*z), 1 - 2 * (x*x + y*y));
				pitch = std::asin(std::clamp(2 * (w*y - z*x), -1.0, 1.0));
				yaw = std::atan2(2 * (w*z + x*y), 1 - 2 * (y*y + z*z));
			}
			else if (validEuler)
			{
				roll = euler.roll; pitch = euler.pitch; yaw = euler.yaw;
				const double cr = std::cos(roll/2), sr = std::sin(roll/2), cp = std::cos(pitch/2),
					sp = std::sin(pitch/2), cy = std::cos(yaw/2), sy = std::sin(yaw/2);
				w = cr*cp*cy + sr*sp*sy; x = sr*cp*cy - cr*sp*sy;
				y = cr*sp*cy + sr*cp*sy; z = cr*cp*sy - sr*sp*cy;
			}
			j["rollRad"] = valid ? json(roll) : json(nullptr);
			j["pitchRad"] = valid ? json(pitch) : json(nullptr);
			j["yawRad"] = valid ? json(yaw) : json(nullptr);
			j["quaternionWxyz"] = valid ? json::array({w, x, y, z}) : json(nullptr);
			j["angularVelocityRadS"] = useQuaternion ? json::array({finite(quaternion.rollspeed), finite(quaternion.pitchspeed), finite(quaternion.yawspeed)}) :
				json::array({finite(euler.rollspeed), finite(euler.pitchspeed), finite(euler.yawspeed)});
			result["attitude"] = std::move(j);
		}

		mavlink_sys_status_t system{};
		if (stream.snapshot<MavSysStatus>(system, timestamp))
		{
			json j = age(timestamp, nowNs, staleAfterMs);
			j["voltageV"] = system.voltage_battery != UINT16_MAX ? json(system.voltage_battery * 0.001) : json(nullptr);
			j["currentA"] = system.current_battery != -1 ? json(system.current_battery * 0.01) : json(nullptr);
			j["remainingPct"] = percent(system.battery_remaining);
			j["loadPct"] = system.load <= 1000 ? json(system.load * 0.1) : json(nullptr);
			result["system"] = j;
			j.erase("loadPct");
			j["source"] = "SYS_STATUS";
			j["temperatureC"] = j["consumedMah"] = j["id"] = nullptr;
			result["battery"] = std::move(j);
		}
		mavlink_battery_status_t battery{};
		if (stream.snapshot<MavBatteryStatus>(battery, timestamp) &&
			(result["battery"].is_null() || timestamp + staleAfterMs * 1000000 > nowNs || result["battery"]["stale"].get<bool>()))
		{
			json j = age(timestamp, nowNs, staleAfterMs);
			uint32_t voltage = 0;
			bool hasVoltage = false;
			for (size_t i = 0; i < 10; ++i)
				if (battery.voltages[i] != UINT16_MAX) { voltage += battery.voltages[i]; hasVoltage = true; }
			for (size_t i = 0; i < 4; ++i)
				if (battery.voltages_ext[i] != 0 && battery.voltages_ext[i] != UINT16_MAX) { voltage += battery.voltages_ext[i]; hasVoltage = true; }
			j["source"] = "BATTERY_STATUS";
			j["id"] = battery.id;
			j["voltageV"] = hasVoltage ? json(voltage * 0.001) : json(nullptr);
			j["currentA"] = battery.current_battery != -1 ? json(battery.current_battery * 0.01) : json(nullptr);
			j["remainingPct"] = percent(battery.battery_remaining);
			j["temperatureC"] = battery.temperature != INT16_MAX ? json(battery.temperature * 0.01) : json(nullptr);
			j["consumedMah"] = battery.current_consumed >= 0 ? json(int32_t(battery.current_consumed)) : json(nullptr);
			result["battery"] = std::move(j);
		}

		mavlink_gps_raw_int_t gps{};
		if (stream.snapshot<MavGpsRawINT>(gps, timestamp))
		{
			json j = age(timestamp, nowNs, staleAfterMs);
			j["valid"] = gps.fix_type >= 3 && coordinates(gps.lat, gps.lon);
			j["fixType"] = gps.fix_type;
			j["satellitesVisible"] = gps.satellites_visible != UINT8_MAX ? json(gps.satellites_visible) : json(nullptr);
			j["hdop"] = gps.eph != UINT16_MAX ? json(gps.eph * 0.01) : json(nullptr);
			j["vdop"] = gps.epv != UINT16_MAX ? json(gps.epv * 0.01) : json(nullptr);
			j["latitudeDeg"] = gps.lat * 1e-7;
			j["longitudeDeg"] = gps.lon * 1e-7;
			j["altitudeMslM"] = gps.alt * 0.001;
			// Zero also denotes an absent MAVLink 2 extension; do not imply known ellipsoid height.
			j["altitudeEllipsoidM"] = gps.alt_ellipsoid != 0 ? json(gps.alt_ellipsoid * 0.001) : json(nullptr);
			result["gps"] = std::move(j);
		}

		mavlink_home_position_t home{};
		if (stream.snapshot<MavHomePosition>(home, timestamp))
		{
			json j = age(timestamp, nowNs, staleAfterMs);
			j["valid"] = coordinates(home.latitude, home.longitude);
			j["latitudeDeg"] = home.latitude * 1e-7;
			j["longitudeDeg"] = home.longitude * 1e-7;
			j["altitudeMslM"] = home.altitude * 0.001;
			result["home"] = std::move(j);
		}

		mavlink_statustext_t status{};
		if (stream.snapshot<MavStatusText>(status, timestamp))
		{
			json j = age(timestamp, nowNs, staleAfterMs);
			j["severity"] = status.severity;
			j["text"] = std::string(status.text, strnlen(status.text, sizeof(status.text)));
			j["id"] = uint16_t(status.id);
			j["chunkSeq"] = status.chunk_seq;
			result["statusText"] = std::move(j);
		}
		return result;
	}
}
