#include "_SporSensor.h"
#include "../../Protocol/_JSONbase.h"
#include "../../UI/_Console.h"
#include <spor/detail/json.hpp>
#include <arpa/inet.h>
#include <cctype>
#include <cmath>
#include <limits>
#include <set>

namespace kai
{
	namespace
	{
		// This catalog is the single source for persistence, input validation and
		// the browser controls. Null device values preserve the sensor's setting.
		const json &sporSchema()
		{
			static const json schema = [] {
				json fields = json::array();
				auto field = [&](const string &key, const string &type, const string &category,
								 const json &value, bool nullable = false, const string &hint = "") -> json & {
					fields.push_back({{"key", key}, {"type", type}, {"category", category},
									  {"default", value}, {"nullable", nullable}, {"hint", hint}});
					return fields.back();
				};
				auto integer = [&](const string &key, const string &category, const json &value,
								   double min, double max, bool nullable = false) -> json & {
					auto &f = field(key, "int", category, value, nullable);
					f["min"] = min; f["max"] = max;
					return f;
				};
				auto number = [&](const string &key, const string &category, double min, double max) -> json & {
					auto &f = field(key, "float", category, nullptr, true);
					f["min"] = min; f["max"] = max;
					return f;
				};
				auto choices = [&](json &f, std::initializer_list<const char *> names) {
					f["choices"] = json::array();
					int index = 0;
					for (const char *name : names)
						f["choices"].push_back({{"value", index++}, {"label", name}});
				};
				field("address", "string", "Connection", "192.168.80.8");
				field("localAddress", "string", "Connection", "", false, "Empty uses the OS route to the sensor.");
				field("sensorId", "string", "Connection", "", false, "Sensor serial/topic ID; recommended when multicast discovery is unreliable.");
				field("protocolVersion", "string", "Connection", "2.6.0.2");
				integer("controlPort", "Connection", 10061, 1, 65535);
				integer("notificationPort", "Connection", 10063, 1, 65535);
				integer("dataPort", "Connection", 10062, 4, 65535);
				integer("publishPort", "Connection", 0, 0, 65535)["hint"] = "0 selects the effective data port minus 3.";
				integer("timeoutMs", "Connection", 3000, 1, 600000);
				integer("streamTimeoutMs", "Connection", 3000, 1, 600000)["hint"] =
					"Profile receive timeout. Increase for infrequent external or software triggers.";
				integer("uiTimeoutMs", "Connection", 8000, 1, 600000);
				integer("reconnectMs", "Connection", 2000, 50, 600000);
				field("ensureSdkServerEnabled", "bool", "Connection", true, false, "Ensure SDK server and scan output are enabled before connecting.");
				field("useDiscoveredPorts", "bool", "Connection", true, false, "Use the ports reported by the sensor; configured native ports are used when disabled.");
				field("autoStart", "bool", "Acquisition", true);
				choices(integer("scanMode", "Acquisition", 1, 0, 2, true), {"Image (no point cloud)", "Profile", "Point cloud"});
				number("scanRateHz", "Acquisition", 0.000001, std::numeric_limits<float>::max())["hint"] =
					"Native rate setter; acknowledgment only. Use either this or verified frameRateHz.";
				number("frameRateHz", "Acquisition", 0.000001, std::numeric_limits<float>::max())["hint"] =
					"Verified frame rate; current sensor limits apply.";
				choices(integer("triggerMode", "Acquisition", nullptr, 0, 3, true),
						{"Continuous", "Encoder", "External input", "Software"});
				field("externalTrigger", "bool", "Acquisition", nullptr, true, "Gate acquisition on the external input.");
				integer("exposureUs", "Exposure and laser", nullptr, 1, UINT32_MAX, true)["hint"] =
					"Single exposure mode, microseconds. Native acknowledgment only; sensor limits apply.";
				choices(integer("laserState", "Exposure and laser", nullptr, 0, 2, true),
						{"Triggered", "Always on", "Off"});
				choices(integer("laserTimingMode", "Exposure and laser", nullptr, 0, 1, true),
						{"Normally", "Strobe"});
				number("laserPower", "Exposure and laser", 0, 100)["hint"] = "Vendor power scale 0–100; not optical watts.";
				field("brightnessEnabled", "bool", "Exposure and laser", nullptr, true, "Collect per-point intensity.");
				field("measurementOutputEnabled", "bool", "SDK output", nullptr, true,
					  "Configure SDK measurement output; scan output stays enabled. Measurement data is not published by this module.");
				for (const char *key : {"areaXRangeMm", "areaZRangeMm", "areaXStartMm", "areaZStartMm"})
					number(key, "Sensor area", string(key).find("Range") != string::npos ? 0.000001 : -1e9, 1e9)["hint"] =
						"Millimetres. Set all four area fields together or preserve all four. Native acknowledgment only.";
				for (const char *key : {"transformXmm", "transformYmm", "transformZmm", "transformXdeg", "transformYdeg", "transformZdeg"})
					number(key, "Sensor transformation", -1e9, 1e9)["hint"] =
						"Manual sensor alignment: translations in millimetres, rotations in degrees.";
				for (const char *kind : {"Gap", "Median", "Smoothing", "Decimation"})
					for (const char *axis : {"X", "Y"})
					{
						const string prefix = string("filter") + kind + axis;
						field(prefix + "Enabled", "bool", "Filters", nullptr, true,
							  "Requires profile/point-cloud mode and uniform spacing configured on the sensor.");
						number(prefix + "Amount", "Filters", 0, std::numeric_limits<float>::max())["hint"] =
							"Sensor units and limits; omitted partner preserves its current value.";
					}
				integer("maxNotificationBytes", "Transport limits", 4 * 1024 * 1024, 1, UINT32_MAX);
				integer("maxMessageBytes", "Transport limits", 64 * 1024 * 1024, 1, UINT32_MAX);
				integer("maxFramesPerMessage", "Transport limits", 4096, 1, UINT32_MAX);
				integer("maxLinesPerGroup", "Transport limits", 64, 1, UINT32_MAX);
				integer("maxPointsPerLine", "Transport limits", 1024 * 1024, 1, UINT32_MAX);
				integer("maxPointsPerMessage", "Transport limits", 4 * 1024 * 1024, 1, UINT32_MAX);
				field("allowUnverifiedCompression", "bool", "Transport limits", false, false,
					  "Opt in to SDK compression formats that have not been verified on this sensor.");
				auto &side = field("profileSide", "string", "Point cloud", "both");
				side["choices"] = json::array({{{"value", "both"}, {"label", "Both"}},
					{{"value", "top"}, {"label", "Top"}}, {{"value", "bottom"}, {"label", "Bottom"}}});
				integer("pointStride", "Point cloud", 1, 1, 1048576);
				auto &color = field("colorMode", "string", "Point cloud", "brightness");
				color["choices"] = json::array({{{"value", "brightness"}, {"label", "Intensity"}},
					{{"value", "solid"}, {"label", "Solid color"}}});
				field("fallbackColor", "array", "Point cloud", json::array({1.0, 1.0, 1.0}), false,
					  "RGB values in [0, 1], used for solid color and profiles without intensity.");
				auto &y = field("yOffsetM", "float", "Point cloud", 0.0, false,
								"Profiles contain X/Z only. Publish Y at this position in metres.");
				y["min"] = -1e6; y["max"] = 1e6;
				for (auto &f : fields)
					f["restart"] = f["category"] != "Point cloud";
				return fields;
			}();
			return schema;
		}

		void validateValue(const json &value, const json &spec)
		{
			const string key = spec["key"], type = spec["type"];
			if (value.is_null() && spec.value("nullable", false)) return;
			bool valid = false;
			if (type == "bool") valid = value.is_boolean();
			else if (type == "int" || type == "float")
				valid = (type != "int" || value.is_number_integer()) && value.is_number() &&
						std::isfinite(value.get<double>()) &&
						value.get<double>() >= spec["min"].get<double>() &&
						value.get<double>() <= spec["max"].get<double>();
			else if (type == "string")
			{
				valid = value.is_string();
				if (valid)
				{
					const string text = value.get<string>();
					valid = text.size() <= 128 && text.find('\0') == string::npos;
					if (valid && (key == "address" || key == "localAddress"))
					{
						in_addr address{};
						valid = (key == "localAddress" && text.empty()) ||
								inet_pton(AF_INET, text.c_str(), &address) == 1;
					}
					if (valid && key == "protocolVersion") valid = !text.empty();
					if (valid && key == "sensorId")
						for (unsigned char c : text)
							if (!std::isalnum(c) && c != '_' && c != '-') valid = false;
				}
			}
			else if (key == "fallbackColor")
			{
				valid = value.is_array() && value.size() == 3;
				if (valid) for (const auto &v : value)
					if (!v.is_number() || !std::isfinite(v.get<double>()) || v < 0 || v > 1) valid = false;
			}
			if (valid && spec.contains("choices"))
			{
				valid = false;
				for (const auto &choice : spec["choices"]) if (choice["value"] == value) valid = true;
			}
			if (!valid) throw std::invalid_argument("Invalid value for " + key);
		}

		json wireJson(const spor::wire::Value &value)
		{
			return json::parse(spor::detail::to_json(value));
		}

		const char *transformKeys[] = {"transformXmm", "transformYmm", "transformZmm",
									  "transformXdeg", "transformYdeg", "transformZdeg"};
	}

	_SporSensor::_SporSensor()
	{
		m_config = json::object();
		for (const auto &spec : sporSchema()) m_config[spec["key"].get<string>()] = spec["default"];
	}

	_SporSensor::~_SporSensor()
	{
		if (m_pT) m_pT->join();
		close();
	}

	bool _SporSensor::loadConfig(void)
	{
		IF_F(!_ModuleBase::loadConfig());
		json patch = json::object(), errors;
		for (const auto &spec : sporSchema())
		{
			const string key = spec["key"];
			if (m_pJ->contains(key)) patch[key] = (*m_pJ)[key];
		}
		if (!applyConfig(patch, false, errors))
		{
			LOG_E("Invalid SporSensor configuration: " + errors.dump());
			return false;
		}
		return true;
	}

	bool _SporSensor::saveConfig(bool bExport)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		IF_F(!_ModuleBase::saveConfig(false));
		m_pJ->update(m_config);
		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _SporSensor::link(InstanceMgr *pM)
	{
		IF_F(!_ModuleBase::link(pM));
		string name;
		jKv(*m_pJ, "PCLframeOut", name);
		m_pPCLout = dynamic_cast<PCLframe *>(static_cast<DataObjBase *>(pM->findDataObject(name)));
		IF_Le_F(!m_pPCLout, "PCLframeOut not found: " + name);
		return true;
	}

	bool _SporSensor::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pPCLout);
		return m_pT->startThread(getUpdate, this);
	}

	bool _SporSensor::check(void)
	{
		NULL_F(m_pPCLout);
		return _ModuleBase::check();
	}

	json _SporSensor::configValues(void) const
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		return m_config;
	}

	json _SporSensor::controlSchema(void) const
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		json schema = sporSchema();
		for (auto &spec : schema)
		{
			const string key = spec["key"];
			if (m_device.contains("values") && m_device["values"].contains(key))
				spec["current"] = m_device["values"][key];
			if (m_device.contains("limits") && m_device["limits"].contains(key))
				spec.update(m_device["limits"][key]);
		}
		return schema;
	}

	bool _SporSensor::applyConfig(const json &patch, bool device, json &errors)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		errors = json::object();
		if (!patch.is_object()) { errors["config"] = "Expected a configuration object"; return false; }
		json candidate = m_config;
		bool reconnect = false;
		for (auto it = patch.begin(); it != patch.end(); ++it)
		{
			bool known = false;
			for (const auto &spec : sporSchema())
			{
				if (spec["key"] != it.key()) continue;
				known = true;
				try
				{
					validateValue(it.value(), spec);
					candidate[it.key()] = it.value();
					if (spec.value("restart", false) && (it.value() != m_config[it.key()] ||
						(device && spec.value("nullable", false) && !it.value().is_null()))) reconnect = true;
				}
				catch (const std::exception &e) { errors[it.key()] = e.what(); }
				break;
			}
			if (!known) errors[it.key()] = "Unknown SporSensor configuration key";
		}
		int areaFields = 0;
		for (const char *key : {"areaXRangeMm", "areaZRangeMm", "areaXStartMm", "areaZStartMm"})
			if (!candidate[key].is_null()) ++areaFields;
		if (areaFields != 0 && areaFields != 4) errors["areaXRangeMm"] = "Set all four sensor area fields, or preserve all four";
		if (!candidate["frameRateHz"].is_null() && !candidate["scanRateHz"].is_null())
			errors["frameRateHz"] = "Use frameRateHz or scanRateHz; preserve the other rate setting";
		if (!errors.empty()) return false;
		const json before = m_config;
		m_config = std::move(candidate);
		if (patch.contains("autoStart")) m_wantAcquisition = m_config["autoStart"].get<bool>();
		if (!device || !reconnect) return true;
		if (!m_opened) { m_nextConnect = 0; return true; }
		close();
		const bool connected = open();
		errors = m_errors;
		// Keep failed settings out of subsequent saves/reconnects. Earlier
		// successful commands can have taken effect; SDK operations are not atomic.
		for (auto it = errors.begin(); it != errors.end(); ++it)
			if (before.contains(it.key())) m_config[it.key()] = before[it.key()];
		if (!connected) errors["connection"] = m_lastError;
		return connected && errors.empty();
	}

	void _SporSensor::applyDeviceSettings(void)
	{
		auto present = [&](const char *key) { return !m_config[key].is_null(); };
		auto apply = [&](std::initializer_list<string> keys, auto operation) {
			bool selected = false;
			for (const auto &key : keys) selected |= !m_config[key].is_null();
			if (!selected) return;
			try { operation(); }
			catch (const std::exception &e)
			{
				for (const auto &key : keys) if (!m_config[key].is_null()) m_errors[key] = e.what();
			}
		};
		apply({"scanMode"}, [&] { m_control->set_scan_mode(m_config["scanMode"].get<int>()); });
		apply({"areaXRangeMm", "areaZRangeMm", "areaXStartMm", "areaZStartMm"}, [&] {
			m_control->set_sensor_area({m_config["areaXRangeMm"].get<double>(), m_config["areaZRangeMm"].get<double>(),
									   m_config["areaXStartMm"].get<double>(), m_config["areaZStartMm"].get<double>()});
		});
		// Intensity can change exposure parameters and the maximum trigger rate.
		apply({"brightnessEnabled"}, [&] { m_settings->set_brightness_enabled(m_config["brightnessEnabled"].get<bool>()); });
		apply({"exposureUs"}, [&] { m_control->set_exposure_us(m_config["exposureUs"].get<uint32_t>()); });
		apply({"laserTimingMode"}, [&] {
			m_settings->set_laser_timing_mode(static_cast<spor::LaserTimingMode>(m_config["laserTimingMode"].get<int>()));
		});
		apply({"laserPower"}, [&] { m_settings->set_laser_power(m_config["laserPower"].get<double>()); });
		apply({"laserState"}, [&] { m_control->set_laser_state(m_config["laserState"].get<int>()); });
		apply({"triggerMode"}, [&] {
			m_settings->set_trigger_mode(static_cast<spor::TriggerMode>(m_config["triggerMode"].get<int>()));
		});
		apply({"externalTrigger"}, [&] { m_control->set_external_trigger(m_config["externalTrigger"].get<bool>()); });
		apply({"scanRateHz"}, [&] { m_control->set_scan_rate(m_config["scanRateHz"].get<float>()); });
		apply({"frameRateHz"}, [&] { m_settings->set_frame_rate(m_config["frameRateHz"].get<double>()); });
		apply({"transformXmm", "transformYmm", "transformZmm", "transformXdeg", "transformYdeg", "transformZdeg"}, [&] {
			auto t = m_settings->transformation();
			double *values[] = {&t.x_mm, &t.y_mm, &t.z_mm, &t.x_degrees, &t.y_degrees, &t.z_degrees};
			for (size_t i = 0; i < 6; ++i) if (present(transformKeys[i])) *values[i] = m_config[transformKeys[i]].get<double>();
			m_settings->set_transformation(t);
		});
		const char *names[] = {"Gap", "Median", "Smoothing", "Decimation"};
		const spor::FilterKind kinds[] = {spor::FilterKind::GapFilling, spor::FilterKind::Median,
										spor::FilterKind::Smoothing, spor::FilterKind::Decimation};
		for (size_t kind = 0; kind < 4; ++kind)
			for (int axis = 0; axis < 2; ++axis)
			{
				const string prefix = string("filter") + names[kind] + (axis ? "Y" : "X");
				const string enabled = prefix + "Enabled", amount = prefix + "Amount";
				apply({enabled, amount}, [&] {
					const auto a = axis ? spor::FilterAxis::Y : spor::FilterAxis::X;
					auto f = m_settings->filter_settings(kinds[kind], a);
					if (!m_config[enabled].is_null()) f.enabled = m_config[enabled].get<bool>();
					if (!m_config[amount].is_null()) f.amount = m_config[amount].get<double>();
					m_settings->set_filter(kinds[kind], a, f.enabled, f.amount);
				});
			}
	}

	void _SporSensor::refreshDevice(void)
	{
		const auto project = m_settings->read_project();
		const json raw = wireJson(project);
		m_device["sdkServer"] = raw.value("sdk_server_state", json::object());
		const string active = raw.at("cur_job").at("scan_setting").at("sensor_group").at("active_device").get<string>();
		m_device["activeDeviceId"] = active;
		m_device["models"] = json::array();
		for (const auto &model : spor::SettingsClient::models_from_project(project))
			m_device["models"].push_back({{"guid", model.guid}, {"deviceId", model.device_id}, {"model", wireJson(model.json)}});
		m_device["values"] = json::object();
		m_device["limits"] = json::object();
		m_device["readbackErrors"] = json::object();
		// Read the same indexed parameter models as SettingsClient, from one
		// consistent project snapshot. Raw models remain available for inspection.
		auto model = [&](const char *guid) -> json {
			json global, local;
			for (const auto &entry : m_device["models"])
				if (entry["guid"] == guid)
				{
					if (entry["deviceId"] == active && !active.empty()) local = entry["model"]["value"];
					else if (entry["deviceId"] == "") global = entry["model"]["value"];
				}
			if (!local.is_null()) return local;
			if (!global.is_null()) return global;
			throw std::runtime_error(string("Parameter model unavailable: ") + guid);
		};
		auto child = [](const json &value, int index) -> json {
			for (const auto &entry : value.at("group"))
				if (entry.at("index") == index) return entry.at("value");
			throw std::runtime_error("Parameter index unavailable: " + std::to_string(index));
		};
		auto value = [&](const string &key, const json &parameter, const char *member = "value") {
			m_device["values"][key] = parameter.at(member);
			if (parameter.contains("range") && parameter["range"].value("use", false))
				m_device["limits"][key] = {{"min", parameter["range"].at("min")}, {"max", parameter["range"].at("max")}};
		};
		auto inspect = [&](const string &name, auto read) {
			try { read(); }
			catch (const std::exception &e) { m_device["readbackErrors"][name] = e.what(); }
		};
		inspect("laser", [&] {
			const auto p = model(spor::SettingsClient::laser_model_guid);
			value("laserTimingMode", child(p, 0)); value("laserEnable", child(p, 1)); value("laserPower", child(p, 2));
		});
		inspect("brightness", [&] { value("brightnessEnabled", model(spor::SettingsClient::brightness_model_guid)); });
		inspect("trigger", [&] {
			const auto p = model(spor::SettingsClient::trigger_model_guid), time = child(p, 2);
			value("triggerMode", child(p, 0), "active_index");
			value("frameRateHz", child(time, 0)); value("scanRateHz", child(time, 0));
			value("externalTrigger", child(time, 1)); value("maximumFrameRateHz", child(p, 1));
		});
		inspect("transformation", [&] {
			const auto p = model(spor::SettingsClient::transformation_model_guid);
			for (int i = 0; i < 6; ++i) value(transformKeys[i], child(p, i));
		});
		const char *names[] = {"Gap", "Median", "Smoothing", "Decimation"};
		const spor::FilterKind kinds[] = {spor::FilterKind::GapFilling, spor::FilterKind::Median,
										spor::FilterKind::Smoothing, spor::FilterKind::Decimation};
		for (size_t kind = 0; kind < 4; ++kind)
			for (int axis = 0; axis < 2; ++axis)
			{
				const string prefix = string("filter") + names[kind] + (axis ? "Y" : "X");
				inspect(prefix, [&] {
					const auto p = model(spor::SettingsClient::filter_model_guid(kinds[kind], axis ? spor::FilterAxis::Y : spor::FilterAxis::X));
					value(prefix + "Enabled", child(p, 0)); value(prefix + "Amount", child(p, 1));
				});
			}
		inspect("measurementOutputEnabled", [&] {
			m_device["values"]["measurementOutputEnabled"] = m_device["sdkServer"].at("data_opt_setting").at("enable_measure_block_data");
		});
		m_device["readbackTimestampNs"] = getTns();
	}

	void _SporSensor::connectStream(void)
	{
		auto stream = std::make_shared<spor::ProfileStream>(m_streamOptions);
		stream->connect();
		m_stream = std::move(stream);
	}

	bool _SporSensor::open(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		if (m_opened) return true;
		m_errors = json::object();
		try
		{
			spor::UiOptions ui;
			ui.address = m_config["address"].get<string>();
			ui.local_address = m_config["localAddress"].get<string>();
			ui.sensor_id = m_config["sensorId"].get<string>();
			ui.timeout = std::chrono::milliseconds(m_config["uiTimeoutMs"].get<int>());
			m_ui = std::make_unique<spor::UiClient>(ui);
			m_ui->connect();
			m_settings = std::make_unique<spor::SettingsClient>(*m_ui);
			auto server = m_config["ensureSdkServerEnabled"].get<bool>() ?
				m_settings->ensure_sdk_server_enabled().settings : m_settings->sdk_server_settings();
			if (!m_config["measurementOutputEnabled"].is_null())
			{
				try { server = m_settings->set_measurement_output(m_config["measurementOutputEnabled"].get<bool>()).settings; }
				catch (const std::exception &e) { m_errors["measurementOutputEnabled"] = e.what(); }
			}

			const bool discovered = m_config["useDiscoveredPorts"].get<bool>();
			spor::Options control;
			control.address = ui.address;
			control.control_port = discovered ? server.control_port : m_config["controlPort"].get<uint16_t>();
			control.notification_port = discovered ? server.notification_port : m_config["notificationPort"].get<uint16_t>();
			control.protocol_version = m_config["protocolVersion"].get<string>();
			control.timeout = std::chrono::milliseconds(m_config["timeoutMs"].get<int>());
			control.max_notification_bytes = m_config["maxNotificationBytes"].get<size_t>();
			m_control = std::make_unique<spor::ControlClient>(control);
			m_control->connect();
			m_control->stop();
			applyDeviceSettings();

			spor::StreamOptions stream;
			stream.address = control.address;
			stream.protocol_version = control.protocol_version;
			stream.timeout = std::chrono::milliseconds(m_config["streamTimeoutMs"].get<int>());
			stream.data_port = discovered ? server.data_port : m_config["dataPort"].get<uint16_t>();
			stream.publish_port = m_config["publishPort"].get<uint16_t>();
			stream.allow_unverified_compression = m_config["allowUnverifiedCompression"].get<bool>();
			stream.limits.max_message_bytes = m_config["maxMessageBytes"].get<size_t>();
			stream.limits.max_frames_per_message = m_config["maxFramesPerMessage"].get<size_t>();
			stream.limits.max_lines_per_group = m_config["maxLinesPerGroup"].get<size_t>();
			stream.limits.max_points_per_line = m_config["maxPointsPerLine"].get<size_t>();
			stream.limits.max_points_per_message = m_config["maxPointsPerMessage"].get<size_t>();
			m_streamOptions = stream;
			connectStream();
			m_device["effectivePorts"] = {{"control", control.control_port}, {"notification", control.notification_port},
										 {"data", stream.data_port}, {"publish", stream.publish_port ? stream.publish_port : stream.data_port - 3}};
			try { refreshDevice(); }
			catch (const std::exception &e) { m_errors["readback"] = e.what(); }
			if (m_wantAcquisition) { m_control->start(); m_acquiring = true; }
			m_opened = true;
			m_lastError.clear();
			return true;
		}
		catch (const std::exception &e)
		{
			m_lastError = e.what();
			LOG_E("SporSensor connection failed: " + m_lastError);
			close();
			m_nextConnect = getTns() + uint64_t(m_config["reconnectMs"].get<int>()) * 1000000;
			return false;
		}
	}

	void _SporSensor::close(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		if (m_control && m_control->connected() && m_acquiring)
		{
			try { m_control->stop(); }
			catch (const std::exception &e) { LOG_E("SporSensor stop was not acknowledged: " + string(e.what())); }
		}
		m_acquiring = false;
		m_opened = false;
		m_stream.reset();
		m_control.reset();
		m_settings.reset();
		m_ui.reset();
		if (m_pPCLout) m_pPCLout->set({});
	}

	void _SporSensor::publishFrame(const spor::ProfileFrame &frame)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		if (!m_pPCLout) return;
		const uint64_t stamp = getTns(); // SDK timestamps have no documented time unit.
		const string side = m_config["profileSide"], colorMode = m_config["colorMode"];
		const size_t stride = m_config["pointStride"].get<size_t>();
		const auto &color = m_config["fallbackColor"];
		const Vector3f fallback(color[0].get<float>(), color[1].get<float>(), color[2].get<float>());
		const float y = m_config["yOffsetM"].get<float>();
		vector<GEOMETRY_POINT> points;
		for (const auto &line : frame.lines)
		{
			if ((side == "top" && line.side != spor::ProfileSide::top) ||
				(side == "bottom" && line.side != spor::ProfileSide::bottom)) continue;
			for (size_t i = 0; i < line.points.size(); i += stride)
			{
				const auto &sample = line.points[i];
				if (!sample.valid || !std::isfinite(sample.x) || !std::isfinite(sample.z) ||
					sample.x == spor::invalid_coordinate || sample.z == spor::invalid_coordinate) continue;
				GEOMETRY_POINT p;
				p.m_vP = Vector3f(float(sample.x * 0.001), y, float(sample.z * 0.001));
				if (!p.m_vP.allFinite()) continue;
				p.m_vC = fallback;
				if (colorMode == "brightness" && line.has_brightness && i < line.brightness.size())
					p.m_vC = Vector3f::Constant(float(line.brightness[i]) / 255.0f);
				p.m_tStamp = stamp;
				points.push_back(p);
			}
		}
		m_pPCLout->set(points, stamp);
		++m_frameCount;
		m_frameId = frame.frame_id;
		m_rawTimestamp = frame.timestamp;
		m_encoder = frame.encoder;
		m_pointCount = points.size();
		m_lastError.clear();
	}

	void _SporSensor::update(void)
	{
		while (m_pT->bRun())
		{
			std::shared_ptr<spor::ProfileStream> stream;
			{
				std::lock_guard<std::recursive_mutex> lock(m_mutex);
				if (!m_opened)
				{
					if (getTns() >= m_nextConnect) open();
				}
				else if (m_acquiring && m_config["scanMode"] != 0) stream = m_stream;
			}
			if (!stream) { m_pT->autoFPS(); continue; }
			// Receive already waits for the sensor. Avoid a second FPS limit
			// here, which would create an ever-growing TCP backlog.
			m_pT->skipSleep();
			m_pT->autoFPS();
			try
			{
				const auto frames = stream->receive();
				std::lock_guard<std::recursive_mutex> lock(m_mutex);
				if (m_stream == stream && m_opened && m_acquiring)
					for (const auto &frame : frames) publishFrame(frame);
			}
			catch (const std::exception &e)
			{
				std::lock_guard<std::recursive_mutex> lock(m_mutex);
				if (m_stream != stream || !m_opened || !m_acquiring) continue;
				m_lastError = e.what();
				if (m_pPCLout) m_pPCLout->set({});
				// Idle triggered acquisition is normal. The SDK closes a
				// timed-out stream, so replace only that subscriber and leave
				// native/UI channels and acquisition running.
				if (m_lastError.find("timed out") != string::npos)
				{
					try { connectStream(); continue; }
					catch (const std::exception &reconnect) { m_lastError = reconnect.what(); }
				}
				LOG_E("SporSensor profile stream: " + m_lastError);
				close();
				m_nextConnect = getTns() + uint64_t(m_config["reconnectMs"].get<int>()) * 1000000;
			}
		}
		close();
	}

	json _SporSensor::status(void) const
	{
		return {{"connected", m_opened}, {"acquiring", m_acquiring}, {"frameCount", m_frameCount},
				{"frameId", m_frameId}, {"rawTimestamp", m_rawTimestamp}, {"encoder", m_encoder},
				{"pointCount", m_pointCount}, {"lastError", m_lastError}, {"errors", m_errors}};
	}

	void _SporSensor::console(void *pConsole)
	{
		if (!pConsole) return;
		_ModuleBase::console(pConsole);
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		auto *console = static_cast<_Console *>(pConsole);
		console->addMsg("SporSensor: " + string(m_opened ? "connected" : "disconnected") +
						", points: " + std::to_string(m_pointCount));
		if (!m_lastError.empty()) console->addMsg(m_lastError);
	}

	void _SporSensor::console(const json &j, void *pJSONbase)
	{
		auto *transport = static_cast<_JSONbase *>(pJSONbase);
		if (!transport || !j.is_object() || !j.contains("cmd") || !j["cmd"].is_string()) return;
		const string cmd = j["cmd"];
		const std::set<string> commands = {"getConfig", "setConfig", "saveConfig", "startAcquisition",
			"stopAcquisition", "softwareTrigger", "resetSensorArea", "setMaximumFrameRate", "readProject", "readModel"};
		if (!commands.count(cmd)) return;
		json reply = {{"cmd", cmd}, {"module", getName()}, {"bSuccess", true}};
		if (j.contains("requestId")) reply["requestId"] = j["requestId"];
		{
			std::lock_guard<std::recursive_mutex> lock(m_mutex);
			try
			{
				if (cmd == "getConfig")
				{
					if (m_opened) refreshDevice();
					reply["actions"] = json::array({
						{{"cmd", "startAcquisition"}, {"label", "Start acquisition"}},
						{{"cmd", "stopAcquisition"}, {"label", "Stop acquisition"}},
						{{"cmd", "softwareTrigger"}, {"label", "Software trigger"}},
						{{"cmd", "resetSensorArea"}, {"label", "Reset sensor area"}},
						{{"cmd", "setMaximumFrameRate"}, {"label", "Use maximum frame rate"}},
						{{"cmd", "readProject"}, {"label", "Read sensor project"}}});
				}
				else if (cmd == "setConfig")
				{
					json errors;
					reply["bSuccess"] = applyConfig(j.value("config", json()), true, errors);
					reply["errors"] = errors;
					reply["pending"] = !m_opened;
				}
				else if (cmd == "saveConfig")
				{
					reply["bSuccess"] = saveConfig(true);
					if (!reply["bSuccess"].get<bool>()) reply["error"] = "Could not save the OpenKAI launch configuration";
				}
				else
				{
					if (!m_opened) throw std::runtime_error("Sensor is not connected");
					if (cmd == "readProject") reply["result"] = wireJson(m_settings->read_project());
					else if (cmd == "readModel")
					{
						if (!j.contains("guid") || !j["guid"].is_string() || j["guid"].get<string>().size() != 36)
							throw std::invalid_argument("readModel requires a model GUID");
						reply["result"] = wireJson(m_settings->read_model(j["guid"].get<string>(), j.value("deviceId", string())).json);
					}
					else if (cmd == "softwareTrigger") m_control->software_trigger();
					else if (cmd == "stopAcquisition")
					{
						m_wantAcquisition = false;
						m_control->stop(); m_acquiring = false;
						m_stream.reset();
						if (m_pPCLout) m_pPCLout->set({});
					}
					else if (cmd == "startAcquisition")
					{
						// A stopped stream may contain old buffered profiles; use a
						// fresh subscriber before starting the next acquisition.
						m_wantAcquisition = true;
						m_control->stop(); m_acquiring = false;
						m_stream.reset();
						connectStream();
						m_control->start(); m_acquiring = true;
					}
					else
					{
						const bool resume = m_acquiring;
						m_control->stop(); m_acquiring = false;
						m_stream.reset();
						try
						{
							if (cmd == "resetSensorArea")
							{
								m_control->reset_sensor_area();
								for (const char *key : {"areaXRangeMm", "areaZRangeMm", "areaXStartMm", "areaZStartMm"}) m_config[key] = nullptr;
							}
							else if (cmd == "setMaximumFrameRate")
							{
								const auto rate = m_settings->set_maximum_frame_rate();
								m_config["frameRateHz"] = rate.frame_rate;
								m_config["scanRateHz"] = nullptr;
							}
							refreshDevice();
						}
						catch (...)
						{
							if (resume) { connectStream(); m_control->start(); m_acquiring = true; }
							throw;
						}
						if (resume)
						{
							connectStream();
							m_control->start(); m_acquiring = true;
						}
					}
				}
			}
			catch (const std::exception &e)
			{
				reply["bSuccess"] = false; reply["error"] = e.what();
				m_lastError = e.what();
				if (m_opened && ((!m_acquiring && m_wantAcquisition) ||
					(m_control && !m_control->connected()) || (m_ui && !m_ui->connected())))
				{
					close();
					m_nextConnect = getTns() + uint64_t(m_config["reconnectMs"].get<int>()) * 1000000;
				}
			}
			reply["schema"] = controlSchema();
			reply["config"] = configValues();
			reply["deviceOpen"] = m_opened;
			reply["status"] = status();
			reply["device"] = m_device;
		}
		// Sending on the console transport must not hold the acquisition lock.
		transport->sendJson(reply);
	}
}
