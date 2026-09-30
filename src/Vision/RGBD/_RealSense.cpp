#include "_RealSense.h"
#include "RealSenseOptions.h"
#include "../../Protocol/_JSONbase.h"
#include <chrono>

namespace kai
{
    namespace
    {
        uint64_t steadyNs()
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        uint64_t frameNs(const rs2::frame &frame)
        {
            const double ns = frame.get_timestamp() * NSEC_MSEC;
            if (!std::isfinite(ns) || ns <= 0 || ns >= static_cast<double>(UINT64_MAX)) return 0;
            return static_cast<uint64_t>(ns);
        }

        int optionPriority(const string &key, const json &value)
        {
            if (key == "RS2_OPTION_VISUAL_PRESET") return 0;
            if (key == "RS2_OPTION_ENABLE_AUTO_EXPOSURE" || key == "RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE")
            {
                const bool enabled = value.is_boolean() ? value.get<bool>() : value.is_number() && value.get<double>() != 0;
                return enabled ? 3 : 1;
            }
            if (key == "RS2_OPTION_HDR_ENABLED" || key == "RS2_OPTION_SEQUENCE_ID" ||
                key == "RS2_OPTION_AUTO_EXPOSURE_LIMIT_TOGGLE" || key == "RS2_OPTION_AUTO_GAIN_LIMIT_TOGGLE") return 1;
            return 2;
        }

    }

    _RealSense::_RealSense()
    {
        m_vSizeRGB = Vector2i(640, 480);
        m_vSizeD = Vector2i(640, 480);
    }

    _RealSense::~_RealSense() { stop(); }

    std::unique_lock<std::recursive_mutex> _RealSense::lockDeviceForWork() const
    {
        // Only outer worker/console entrypoints use admission. A waiter holds
        // it until the current frame releases the device, preventing the frame
        // loop from immediately reacquiring an unfair mutex when over budget.
        // Nested config/open/close calls use the recursive device mutex directly.
        std::lock_guard<std::mutex> admission(m_deviceAdmission);
        return std::unique_lock<std::recursive_mutex>(m_deviceMutex);
    }

    json _RealSense::configValues() const
    {
        std::lock_guard<std::recursive_mutex> lock(m_deviceMutex);
        return {{"SN", m_SN}, {"devFPS", m_devFPS}, {"devFPSd", m_devFPSd},
            {"accelFPS", m_accelFPS}, {"gyroFPS", m_gyroFPS}, {"tOutMs", m_tOutMs},
            {"vSizeRGB", {m_vSizeRGB.x(), m_vSizeRGB.y()}}, {"vSizeD", {m_vSizeD.x(), m_vSizeD.y()}},
            {"vRangeD", {m_vRangeD.x(), m_vRangeD.y()}}, {"dOfs", m_dOfs},
            {"bRGB", m_bRGB}, {"bDepth", m_bDepth}, {"bIR", m_bIR}, {"bIMU", m_bIMU},
            {"bPCL", m_bPCL}, {"bPCLrgb", m_bPCLrgb}, {"bAlign", m_bAlign},
            {"bDecimation", m_bDecimation}, {"bSpatial", m_bSpatial}, {"bTemporal", m_bTemporal},
            {"bHoleFilling", m_bHoleFilling}, {"bThreshold", m_bThreshold}, {"sensorOptions", m_sensorOptions}};
    }

    void _RealSense::setConfigValues(const json &j)
    {
        m_SN = j.at("SN").get<string>();
        m_devFPS = j.at("devFPS"); m_devFPSd = j.at("devFPSd");
        m_accelFPS = j.at("accelFPS"); m_gyroFPS = j.at("gyroFPS"); m_tOutMs = j.at("tOutMs");
        m_vSizeRGB = Vector2i(j.at("vSizeRGB")[0].get<int>(), j.at("vSizeRGB")[1].get<int>());
        m_vSizeD = Vector2i(j.at("vSizeD")[0].get<int>(), j.at("vSizeD")[1].get<int>());
        m_vRangeD = Vector2f(j.at("vRangeD")[0].get<float>(), j.at("vRangeD")[1].get<float>());
        m_dOfs = j.at("dOfs");
        m_bRGB = j.at("bRGB"); m_bDepth = j.at("bDepth"); m_bIR = j.at("bIR"); m_bIMU = j.at("bIMU");
        m_bPCL = j.at("bPCL"); m_bPCLrgb = j.at("bPCLrgb"); m_bAlign = j.at("bAlign");
        m_bDecimation = j.at("bDecimation"); m_bSpatial = j.at("bSpatial"); m_bTemporal = j.at("bTemporal");
        m_bHoleFilling = j.at("bHoleFilling"); m_bThreshold = j.at("bThreshold");
        m_sensorOptions = j.at("sensorOptions");
    }

    bool _RealSense::loadConfig()
    {
        std::lock_guard<std::recursive_mutex> lock(m_deviceMutex);
        IF_F(!_ModuleBase::loadConfig());
        // Camera settings use the same validated schema as console updates.
        json patch = json::object(), errors;
        const json values = configValues();
        for (const auto &entry : values.items())
            if (m_pJ->contains(entry.key())) patch[entry.key()] = (*m_pJ)[entry.key()];
        if (!applyConfig(patch, false, errors)) { LOG_E(errors.dump()); return false; }
        DEL(m_pTpp);
        m_pTpp = createThread(jK(*m_pJ, "threadPP"), "threadPP");
        return m_pTpp != nullptr;
    }

    bool _RealSense::saveConfig(bool bExport)
    {
        std::lock_guard<std::recursive_mutex> lock(m_deviceMutex);
        IF_F(!_ModuleBase::saveConfig(false));
        IF_F(m_pTpp && !m_pTpp->saveConfig(false));
        m_pJ->update(configValues());
        return !bExport || m_pJcfg->saveToFile();
    }

    std::map<string, rs2::options> _RealSense::optionTargets()
    {
        std::map<string, rs2::options> targets;
        for (const auto &entry : m_sensors) targets.emplace(entry.first, entry.second);
        targets.emplace("decimation", m_decimation); targets.emplace("spatial", m_spatial);
        targets.emplace("temporal", m_temporal); targets.emplace("holeFilling", m_holeFilling);
        targets.emplace("threshold", m_threshold);
        return targets;
    }

    void _RealSense::discoverOptions()
    {
        for (auto &target : optionTargets())
            for (auto option : target.second.get_supported_options())
            {
                const string key = realsense::optionName(option);
                try
                {
                    if (!target.second.is_option_read_only(option)) m_preStreamWritable.insert(target.first + "." + key);
                }
                catch (const std::exception &) {}
                if (!m_sensorOptions.contains(target.first)) m_sensorOptions[target.first] = json::object();
                if (!m_sensorOptions[target.first].contains(key)) m_sensorOptions[target.first][key] = nullptr;
            }
    }

    void _RealSense::applyOptions()
    {
        auto targets = optionTargets();
        for (const auto &domain : m_sensorOptions.items())
        {
            // Disable automatic modes before manual values: toggling auto off
            // can restore the camera's cached exposure. Enable auto modes last.
            vector<string> keys;
            for (const auto &entry : domain.value().items()) if (!entry.value().is_null()) keys.push_back(entry.key());
            std::stable_sort(keys.begin(), keys.end(), [&](const string &a, const string &b) {
                return optionPriority(a, domain.value()[a]) < optionPriority(b, domain.value()[b]);
            });
            for (const auto &key : keys)
            {
                try
                {
                    auto target = targets.find(domain.key());
                    if (target == targets.end()) throw std::invalid_argument("Sensor is unavailable on this device");
                    realsense::setOption(target->second, realsense::optionId(key), domain.value()[key]);
                }
                catch (const std::exception &e) { throw std::runtime_error(domain.key() + "." + key + ": " + e.what()); }
            }
        }
    }

    bool _RealSense::applyConfig(const json &patch, bool live, json &errors)
    {
        std::lock_guard<std::recursive_mutex> lock(m_deviceMutex);
        errors = json::object();
        if (!patch.is_object()) { errors["config"] = "Expected an object"; return false; }
        const json before = configValues();
        json next = before;
        // rs2::options is a non-owning view. Retain the sensor/filter owners
        // through close/reopen and rollback so their option handles stay valid.
        const auto sensorsBefore = m_sensors;
        const auto decimationBefore = m_decimation;
        const auto spatialBefore = m_spatial;
        const auto temporalBefore = m_temporal;
        const auto holeFillingBefore = m_holeFilling;
        const auto thresholdBefore = m_threshold;
        auto targets = optionTargets();
        const std::set<string> domains = {"depth", "color", "motion", "decimation", "spatial", "temporal", "holeFilling", "threshold"};
        for (const auto &entry : patch.items())
        {
            const auto &key = entry.key(); const auto &v = entry.value();
            try
            {
                if (!before.contains(key)) throw std::invalid_argument("Unknown parameter");
                if (key == "sensorOptions")
                {
                    if (!v.is_object()) throw std::invalid_argument("Expected an object of option domains");
                    for (const auto &domain : v.items())
                    {
                        if (!domains.count(domain.key()) || !domain.value().is_object()) throw std::invalid_argument("Unknown domain or invalid options: " + domain.key());
                        for (const auto &opt : domain.value().items())
                        {
                            const string path = domain.key() + "." + opt.key();
                            try
                            {
                                const auto id = realsense::optionId(opt.key());
                                if (id == RS2_OPTION_COUNT) throw std::invalid_argument("Unknown SDK option");
                                if (!opt.value().is_null())
                                {
                                    if (!opt.value().is_number() && !opt.value().is_boolean() && !opt.value().is_string() && !opt.value().is_array())
                                        throw std::invalid_argument("Expected a number, boolean, string, rectangle array or null");
                                    auto target = targets.find(domain.key());
                                    if (target != targets.end()) realsense::validateOption(target->second, id, opt.value(), !m_preStreamWritable.count(path));
                                    else if (live && m_bOpened) throw std::invalid_argument("Sensor is unavailable");
                                }
                                next[key][domain.key()][opt.key()] = opt.value();
                            }
                            catch (const std::exception &e) { errors[path] = e.what(); }
                        }
                    }
                    continue;
                }
                if (before[key].is_boolean() && !v.is_boolean()) throw std::invalid_argument("Expected a boolean");
                if (before[key].is_string() && !v.is_string()) throw std::invalid_argument("Expected a string");
                if (before[key].is_number_integer())
                {
                    const int minimum = key == "accelFPS" || key == "gyroFPS" ? 0 : 1;
                    const int maximum = key == "tOutMs" ? 60000 : 2000;
                    if (!v.is_number_integer() || v.get<double>() < minimum || v.get<double>() > maximum) throw std::invalid_argument("Integer outside supported bounds");
                }
                if (key == "dOfs" && (!v.is_number() || !std::isfinite(v.get<float>()))) throw std::invalid_argument("Expected a finite distance");
                if (before[key].is_array())
                {
                    if (!v.is_array() || v.size() != 2) throw std::invalid_argument("Expected two numbers");
                    for (const auto &n : v)
                    {
                        if (!n.is_number() || !std::isfinite(n.get<float>())) throw std::invalid_argument("Expected finite numbers");
                        if (key != "vRangeD" && (!n.is_number_integer() || n.get<double>() < 1 || n.get<double>() > 16384)) throw std::invalid_argument("Invalid image dimensions");
                    }
                    if (key == "vRangeD" && (v[0].get<double>() < 0 || v[1].get<double>() <= v[0].get<double>())) throw std::invalid_argument("Expected 0 <= minimum < maximum");
                }
                next[key] = v;
            }
            catch (const std::exception &e) { errors[key] = e.what(); }
        }
        if (!next["bRGB"].get<bool>() && (next["bAlign"].get<bool>() || next["bPCLrgb"].get<bool>())) errors["bRGB"] = "RGB is required for alignment or colored points";
        if (!next["bDepth"].get<bool>() && (next["bAlign"].get<bool>() || next["bPCL"].get<bool>() || next["bPCLrgb"].get<bool>())) errors["bDepth"] = "Depth is required for alignment or point clouds";
        if (!next["bRGB"].get<bool>() && !next["bDepth"].get<bool>() && !next["bIR"].get<bool>() && !next["bIMU"].get<bool>()) errors["config"] = "Enable at least one stream";
        if (!errors.empty()) return false;
        if (before == next) return true;
        const bool reopen = live && m_bOpened;
        json hardwareBefore = json::object();
        if (reopen)
        {
            for (auto &target : targets)
                for (auto id : target.second.get_supported_options())
                    try
                    {
                        if (!target.second.is_option_read_only(id) || m_preStreamWritable.count(target.first + "." + realsense::optionName(id)))
                            hardwareBefore[target.first][realsense::optionName(id)] = realsense::optionValue(target.second, id);
                    }
                    catch (const std::exception &) {} // Volatile/unavailable telemetry is not restorable.
            close();
        }
        setConfigValues(next);
        // Reopen for hardware changes: some SDK options are only writable before
        // streaming. A failed profile or option rolls back the requested config.
        if (reopen && !open())
        {
            errors["device"] = m_lastError;
            setConfigValues(before);
            // open() has released stream ownership; restore values changed by
            // earlier setters before reapplying the original requested config.
            for (auto &target : targets)
            {
                if (!hardwareBefore.contains(target.first)) continue;
                auto restore = [&](const string &key) {
                    try
                    {
                        const auto id = realsense::optionId(key);
                        const auto &value = hardwareBefore[target.first][key];
                        if (realsense::optionValue(target.second, id) != value)
                            realsense::setOption(target.second, id, value);
                    }
                    catch (const std::exception &e) { errors["rollback." + target.first + "." + key] = e.what(); }
                };
                const auto &saved = hardwareBefore[target.first];
                vector<string> keys;
                for (const auto &option : saved.items()) keys.push_back(option.key());
                std::stable_sort(keys.begin(), keys.end(), [&](const string &a, const string &b) {
                    return optionPriority(a, saved[a]) < optionPriority(b, saved[b]);
                });
                for (const auto &key : keys) restore(key);
            }
            if (!open()) errors["rollback"] = m_lastError;
            return false;
        }
        return true;
    }

    json _RealSense::controlSchema()
    {
        std::lock_guard<std::recursive_mutex> lock(m_deviceMutex);
        json schema = json::array();
        const json values = configValues();
        for (const auto &entry : values.items())
        {
            if (entry.key() == "sensorOptions") continue;
            const auto &v = entry.value();
            json field = {{"key", entry.key()}, {"category", "Streams and point cloud"}, {"nullable", false}, {"restart", true},
                {"type", v.is_boolean() ? "bool" : v.is_string() ? "string" : v.is_array() ? (entry.key() == "vRangeD" ? "range" : "size") : v.is_number_integer() ? "int" : "float"}};
            if (v.is_number_integer()) { field["min"] = (entry.key() == "accelFPS" || entry.key() == "gyroFPS") ? 0 : 1; field["max"] = entry.key() == "tOutMs" ? 60000 : 2000; }
            schema.push_back(field);
        }
        auto targets = optionTargets();
        for (const auto &domain : m_sensorOptions.items())
            for (const auto &entry : domain.value().items())
            {
                auto target = targets.find(domain.key());
                json field = {{"key", entry.key()}, {"type", "float"}, {"nullable", true}, {"supported", false}};
                if (target != targets.end() && target->second.supports(realsense::optionId(entry.key())))
                    try { field = realsense::optionSchema(target->second, realsense::optionId(entry.key())); }
                    catch (const std::exception &e) { field["note"] = e.what(); }
                if (m_preStreamWritable.count(domain.key() + "." + entry.key())) field["readOnly"] = false;
                field["domain"] = domain.key(); field["category"] = domain.key(); field["restart"] = true;
                schema.push_back(field);
            }
        return schema;
    }

    bool _RealSense::open()
    {
        std::lock_guard<std::recursive_mutex> lock(m_deviceMutex);
        if (m_bOpened) return true;
        try
        {
            rs2::config config;
            config.disable_all_streams();
            if (!m_SN.empty()) config.enable_device(m_SN);
            if (m_bDepth) config.enable_stream(RS2_STREAM_DEPTH, m_vSizeD.x(), m_vSizeD.y(), RS2_FORMAT_Z16, m_devFPSd);
            if (m_bRGB) config.enable_stream(RS2_STREAM_COLOR, m_vSizeRGB.x(), m_vSizeRGB.y(), RS2_FORMAT_BGR8, m_devFPS);
            if (m_bIR) config.enable_stream(RS2_STREAM_INFRARED, 1, m_vSizeD.x(), m_vSizeD.y(), RS2_FORMAT_Y8, m_devFPSd);
            if (m_bIMU)
            {
                config.enable_stream(RS2_STREAM_ACCEL, RS2_FORMAT_MOTION_XYZ32F, m_accelFPS);
                config.enable_stream(RS2_STREAM_GYRO, RS2_FORMAT_MOTION_XYZ32F, m_gyroFPS);
            }
            m_profile = config.resolve(m_pipeline);
            m_sensors.clear();
            for (auto sensor : m_profile.get_device().query_sensors())
            {
                string domain;
                for (const auto &profile : sensor.get_stream_profiles())
                {
                    if (profile.stream_type() == RS2_STREAM_DEPTH) { domain = "depth"; break; }
                    if (profile.stream_type() == RS2_STREAM_COLOR) domain = "color";
                    if (profile.stream_type() == RS2_STREAM_ACCEL || profile.stream_type() == RS2_STREAM_GYRO) domain = "motion";
                }
                if (!domain.empty()) m_sensors.emplace(domain, sensor);
            }
            // Stateful processing blocks must not carry frames across a restart.
            m_temporal = rs2::temporal_filter(); m_align = rs2::align(RS2_STREAM_COLOR);
            m_pointcloud = rs2::pointcloud();
            m_preStreamWritable.clear();
            discoverOptions();
            applyOptions();
            m_lastVideo = m_lastAccel = m_lastGyro = steadyNs();
            m_profile = m_pipeline.start(config, [this](rs2::frame frame) { receiveFrame(std::move(frame)); });
            m_pipelineStarted = true;
            if (m_bDepth) m_dScale = m_profile.get_device().first<rs2::depth_sensor>().get_depth_scale();
            m_bOpened = true;
            m_lastError.clear();
            LOG_I(string("RealSense opened: ") + m_profile.get_device().get_info(RS2_CAMERA_INFO_NAME));
            return true;
        }
        catch (const std::exception &e)
        {
            m_lastError = e.what(); LOG_E("RealSense open: " + m_lastError);
            close();
            return false;
        }
    }

    void _RealSense::close()
    {
        std::lock_guard<std::recursive_mutex> lock(m_deviceMutex);
        if (m_pipelineStarted)
        {
            try { m_pipeline.stop(); } catch (const std::exception &e) { LOG_E(e.what()); }
            m_pipelineStarted = false;
        }
        rs2::frame old;
        while (m_frames.poll_for_frame(&old)) {}
        m_sensors.clear();
        m_imuPreview.reset();
        _RGBDbase::close();
    }

    void _RealSense::receiveFrame(rs2::frame frame)
    {
        // SDK callbacks may run on different sensor threads. Never acquire the
        // device mutex here: stop() holds it while joining SDK callbacks.
        try
        {
            if (auto motion = frame.as<rs2::motion_frame>())
            {
                const auto v = motion.get_motion_data();
                const uint64_t stamp = frameNs(motion);
                if (motion.get_profile().stream_type() == RS2_STREAM_ACCEL)
                {
                    m_lastAccel = steadyNs(); ++m_nAccel;
                    if (m_pIMUout) m_pIMUout->addAcc({v.x, v.y, v.z}, stamp);
                    m_imuPreview.addAcc({v.x, v.y, v.z}, stamp);
                }
                else if (motion.get_profile().stream_type() == RS2_STREAM_GYRO)
                {
                    m_lastGyro = steadyNs(); ++m_nGyro;
                    if (m_pIMUout) m_pIMUout->addGyro({v.x, v.y, v.z}, stamp);
                    m_imuPreview.addGyro({v.x, v.y, v.z}, stamp);
                }
            }
            else if (auto frames = frame.as<rs2::frameset>())
            {
                m_lastVideo = steadyNs(); ++m_nVideo;
                m_frames.enqueue(std::move(frames));
            }
        }
        catch (const std::exception &e) { LOG_E(string("RealSense callback: ") + e.what()); }
    }

    bool _RealSense::start()
    {
        if (!m_pT || !m_pTpp) return false;
        if (!m_pTpp->startThread(getTPP, this)) return false;
        if (!m_pT->startThread(getUpdate, this)) { m_pTpp->join(); return false; }
        return true;
    }

    void _RealSense::stop()
    {
        if (m_pT) m_pT->stop();
        if (m_pTpp) m_pTpp->stop();
        if (m_pT) m_pT->join();
        if (m_pTpp) m_pTpp->join();
        close();
    }

    void _RealSense::update()
    {
        while (m_pT->bRun())
        {
            bool retry = false;
            {
                auto lock = lockDeviceForWork();
                if (!m_bOpened) retry = !open();
                else
                {
                    const uint64_t now = steadyNs(), timeout = uint64_t(m_tOutMs) * NSEC_MSEC;
                    // A callback can publish a newer time after 'now' is read.
                    // Guard subtraction so that race cannot underflow into a timeout.
                    const auto expired = [now, timeout](uint64_t last) { return now > last && now - last > timeout; };
                    if (((m_bRGB || m_bDepth || m_bIR) && expired(m_lastVideo.load())) ||
                        (m_bIMU && (expired(m_lastAccel.load()) || expired(m_lastGyro.load()))))
                    {
                        m_lastError = "Camera stream timed out"; LOG_E(m_lastError); close(); retry = true;
                    }
                }
            }
            if (retry) m_pT->sleepT(NSEC_SEC);
            m_pT->autoFPS();
        }
    }

    void _RealSense::updateTPP()
    {
        while (m_pTpp->bRun())
        {
            {
                auto lock = lockDeviceForWork();
                rs2::frameset frames;
                if (m_bOpened && m_frames.poll_for_frame(&frames))
                    try { processFrames(std::move(frames)); }
                    catch (const std::exception &e) { LOG_E(string("RealSense processing: ") + e.what()); }
            }
            m_pTpp->autoFPS();
        }
    }

    void _RealSense::processFrames(rs2::frameset frames)
    {
        rs2::frame color = frames.get_color_frame();
        rs2::frame depth = frames.get_depth_frame();
        Mat rgb;
        if (color)
        {
            const auto video = color.as<rs2::video_frame>();
            rgb = Mat(video.get_height(), video.get_width(), CV_8UC3, const_cast<void *>(color.get_data()), video.get_stride_in_bytes());
            if (m_pRGBout) m_pRGBout->set(rgb, frameNs(color));
        }
        if (m_bIR && m_pIRout)
            if (auto ir = frames.get_infrared_frame(1))
            {
                Mat pixels(ir.get_height(), ir.get_width(), CV_8UC1, const_cast<void *>(ir.get_data()), ir.get_stride_in_bytes());
                m_pIRout->set(pixels, frameNs(ir));
            }
        if (!depth) return;
        if (m_bDecimation) depth = m_decimation.process(depth);
        if (m_bThreshold) depth = m_threshold.process(depth);
        if (m_bSpatial) depth = m_spatial.process(depth);
        if (m_bTemporal) depth = m_temporal.process(depth);
        if (m_bHoleFilling) depth = m_holeFilling.process(depth);
        const uint64_t stamp = frameNs(depth);
        // Points always use the depth optical frame, including when display
        // alignment is enabled. GLIM's IMU extrinsics therefore stay valid.
        updatePC(depth, color, rgb, stamp);
        if (m_bAlign && color)
        {
            // Keep display alignment and point clouds on the same filtered
            // measurement. A composite frame retains the original color and
            // replaces only depth, including the decimated camera intrinsics.
            rs2::filter compose([depth, color](rs2::frame, rs2::frame_source &source) {
                source.frame_ready(source.allocate_composite_frame({depth, color}));
            });
            depth = m_align.process(compose.process(frames).as<rs2::frameset>()).get_depth_frame();
        }
        const auto video = depth.as<rs2::video_frame>();
        Mat raw(video.get_height(), video.get_width(), CV_16UC1, const_cast<void *>(depth.get_data()), video.get_stride_in_bytes());
        Mat distance;
        raw.convertTo(distance, CV_32FC1, depth.as<rs2::depth_frame>().get_units(), m_dOfs);
        distance.setTo(0, raw == 0);
        if (m_pDout) m_pDout->set(distance, stamp);
        if (!rgb.empty())
        {
            if (m_pRGBDout) m_pRGBDout->set(rgb, distance, stamp);
            if (m_bAlign && m_pRGBDtRGBout) m_pRGBDtRGBout->set(rgb, distance, stamp);
        }
    }

    void _RealSense::updatePC(const rs2::frame &depth, const rs2::frame &color, const Mat &rgb, uint64_t stamp)
    {
        if (!m_pPCLout || (!m_bPCL && !m_bPCLrgb)) return;
        if (m_bPCLrgb && color) m_pointcloud.map_to(color);
        const rs2::points points = m_pointcloud.calculate(depth);
        const auto *vertices = points.get_vertices();
        const auto *uvs = points.get_texture_coordinates();
        vector<GEOMETRY_POINT> cloud;
        cloud.reserve(points.size());
        for (size_t i = 0; i < points.size(); ++i)
        {
            const auto &p = vertices[i];
            if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) || p.z <= 0) continue;
            const float z = p.z + m_dOfs;
            if (z <= 0 || z < m_vRangeD.x() || z > m_vRangeD.y()) continue;
            Vector3f point(p.x * z / p.z, p.y * z / p.z, z), c(1, 1, 1);
            if (m_bPCLrgb && !rgb.empty())
            {
                const auto &uv = uvs[i];
                if (std::isfinite(uv.u) && std::isfinite(uv.v) && uv.u >= 0 && uv.u < 1 && uv.v >= 0 && uv.v < 1)
                {
                    const Vec3b pixel = rgb.at<Vec3b>(int(uv.v * rgb.rows), int(uv.u * rgb.cols));
                    c = Vector3f(pixel[2], pixel[1], pixel[0]) / 255.0f;
                }
            }
            cloud.push_back({point, c, stamp});
        }
        m_pPCLout->set(cloud, stamp);
    }

    json _RealSense::imuValues() const
    {
        // Called with the device lock held; SDK callbacks only take the preview
        // mutex. Reading this snapshot never consumes the IMUstream used by SLAM.
        const auto sample = m_imuPreview.snapshot();
        const uint64_t now = steadyNs();
        const auto fresh = [now](uint64_t last) {
            return last && (last >= now || now - last <= NSEC_SEC / 2);
        };
        const bool available = m_bOpened && m_bIMU && sample.tGyro && sample.tAcc &&
            fresh(m_lastGyro.load()) && fresh(m_lastAccel.load());
        return {{"enabled", m_bIMU}, {"deviceOpen", m_bOpened}, {"available", available},
            // Strings preserve capture-clock nanoseconds beyond JS integer precision.
            {"tGyro", std::to_string(sample.tGyro)}, {"tAcc", std::to_string(sample.tAcc)},
            {"tFusion", std::to_string(sample.tFusion)},
            {"gyro", {sample.gyro.x(), sample.gyro.y(), sample.gyro.z()}},
            {"acc", {sample.acc.x(), sample.acc.y(), sample.acc.z()}},
            {"quaternion", {sample.quaternion.w(), sample.quaternion.x(), sample.quaternion.y(), sample.quaternion.z()}},
            {"rpy", {sample.rpy.x(), sample.rpy.y(), sample.rpy.z()}},
            {"fusion", true}, {"orientationValid", available && sample.orientationValid}};
    }

    void _RealSense::console(void *pConsole)
    {
        auto lock = lockDeviceForWork();
        _RGBDbase::console(pConsole);
        if (pConsole) static_cast<_Console *>(pConsole)->addMsg("RealSense frames=" + std::to_string(m_nVideo.load()) +
            " accel=" + std::to_string(m_nAccel.load()) + " gyro=" + std::to_string(m_nGyro.load()));
    }

    void _RealSense::console(const json &j, void *pJSONbase)
    {
        auto lock = lockDeviceForWork();
        auto *endpoint = static_cast<_JSONbase *>(pJSONbase);
        if (!endpoint || !j.is_object() || !j.contains("cmd") || !j["cmd"].is_string()) return;
        const string cmd = j["cmd"].get<string>();
        if (cmd != "getConfig" && cmd != "setConfig" && cmd != "saveConfig" && cmd != "getIMU") return;
        json reply = {{"cmd", cmd}, {"module", getName()}, {"bSuccess", true}};
        if (j.contains("requestId")) reply["requestId"] = j["requestId"];
        try
        {
            if (cmd == "getIMU") reply.update(imuValues());
            else
            {
                if (cmd == "setConfig")
                {
                    json errors;
                    reply["bSuccess"] = applyConfig(j.value("config", json()), true, errors);
                    reply["errors"] = errors;
                }
                else if (cmd == "saveConfig") reply["bSuccess"] = saveConfig(true);
                reply["config"] = configValues(); reply["schema"] = controlSchema();
                reply["deviceOpen"] = m_bOpened;
                reply["status"] = {{"videoFrames", m_nVideo.load()}, {"accelSamples", m_nAccel.load()}, {"gyroSamples", m_nGyro.load()}, {"lastError", m_lastError}};
                if (!m_lastError.empty()) reply["error"] = m_lastError;
            }
        }
        catch (const std::exception &e) { reply["bSuccess"] = false; reply["error"] = e.what(); }
        endpoint->sendJson(reply);
    }
}
