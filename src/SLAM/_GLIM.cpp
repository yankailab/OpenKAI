/*
 * _GLIM.cpp
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#include "_GLIM.h"
#include "../Universe/Geometry/PointCloud/_PCfile.h"

#include <filesystem>
#include <glim/util/config.hpp>
#include <glim/preprocess/cloud_preprocessor.hpp>
#include <glim/odometry/odometry_estimation_base.hpp>
#include <glim/odometry/odometry_estimation_cpu.hpp>
#include <glim/odometry/odometry_estimation_ct.hpp>
#include <glim/mapping/sub_mapping_base.hpp>
#include <glim/mapping/sub_mapping.hpp>
#include <glim/mapping/global_mapping_base.hpp>
#include <glim/mapping/global_mapping.hpp>

namespace kai
{
	namespace
	{
		// GLIM's configuration is process-wide. Never replace it beneath a session.
		std::mutex configMutex;
		string configuredPath;

		// Native SubMapping delays one input for IMU interpolation, including when
		// IMU is disabled. Its end-of-sequence API does not drain that lookahead.
		class IMUFreeSubMapping : public glim::SubMapping
		{
		public:
			explicit IMUFreeSubMapping(const glim::SubMappingParams &params) : glim::SubMapping(params) {}

			void insert_frame(const glim::EstimationFrame::ConstPtr &frame) override
			{
				glim::SubMapping::insert_frame(frame);
				m_lastFrame = frame;
			}

			vector<glim::SubMap::Ptr> submit_end_of_sequence() override
			{
				// A repeated final frame supplies lookahead only. The real final
				// frame is processed once; the sentinel dies with this mapper.
				auto last = std::move(m_lastFrame);
				if (last) glim::SubMapping::insert_frame(last);
				auto submaps = glim::SubMapping::get_submaps();
				const auto tail = glim::SubMapping::submit_end_of_sequence();
				submaps.insert(submaps.end(), tail.begin(), tail.end());
				return submaps;
			}

		private:
			glim::EstimationFrame::ConstPtr m_lastFrame;
		};

		glim::Config readConfig(const string &name)
		{
			const string path = glim::GlobalConfig::get_config_path(name);
			if (!std::filesystem::is_regular_file(path))
				throw std::runtime_error("Missing GLIM config: " + path);
			return glim::Config(path);
		}
	}

	_GLIM::_GLIM() = default;

	_GLIM::~_GLIM()
	{
		// Join the OpenKAI worker before destroying GLIM or base-class state.
		if (m_pT)
			m_pT->join();
		stopTracking();
	}

	bool _GLIM::loadConfig(void)
	{
		IF_F(!_SLAMbase::loadConfig());
		const json &j = *m_pJ;
		jKv(j, "configPath", m_configPath);
		jKv(j, "bMapping", m_bMapping);
		jKv(j, "nMinPoints", m_nMinPoints);
		jKv(j, "nMapPoints", m_nMapPoints);
		jKv(j, "nLiveFrames", m_nLiveFrames);
		jKv(j, "tMapUpdateNs", m_mapIntervalNs);
		jKv(j, "bPublishLiveMap", m_bPublishLiveMap);
		jKv(j, "exportPath", m_exportPath);
		IF_Le_F(m_configPath.empty(), "GLIM configPath is required");
		IF_Le_F(m_nMinPoints < 10, "GLIM nMinPoints must be at least 10");
		IF_Le_F(m_nMapPoints < 1 || m_nLiveFrames < 1, "GLIM map limits must be positive");
		try
		{
			m_parameters = parameterDefaults();
			applyParameters(validatedParameters(j.value("parameters", json::object())));
		}
		catch (const std::exception &e) { LOG_E(e.what()); return false; }
		m_session = getTns();
		return true;
	}

	bool _GLIM::link(void)
	{
		IF_F(!_SLAMbase::link());
		const json &j = *m_pJ;
		string name;
		jKv(j, "globalMapPCL", name);
		m_pGlobalMap = name.empty() ? nullptr : dynamic_cast<_PointCloud *>(static_cast<BASE *>(m_pM->findModule(name)));
		IF_Le_F(!name.empty() && !m_pGlobalMap, "Cannot find globalMapPCL: " + name);
		IF_Le_F(m_pGlobalMap && m_pGlobalMap == m_pPCL, "GLIM input and globalMapPCL must be different buffers");
		return true;
	}

	json _GLIM::parameterDefaults() const
	{
		std::ifstream input(std::filesystem::path(m_configPath) / "config.json");
		if (!input) throw std::runtime_error("Cannot read GLIM config.json: " + m_configPath);
		const auto root = json::parse(input, nullptr, true, true).at("global");
		auto config = [&](const string &name) {
			return glim::Config((std::filesystem::path(m_configPath) / root.value(name, name + ".json")).string());
		};
		const auto p = config("config_preprocess");
		json result = {{"bMapping", m_bMapping}, {"nMinPoints", m_nMinPoints},
			{"preprocess", {
				{"distanceNear", p.param<double>("preprocess", "distance_near_thresh", 1.0)},
				{"distanceFar", p.param<double>("preprocess", "distance_far_thresh", 100.0)},
				{"voxelResolution", p.param<double>("preprocess", "downsample_resolution", .15)},
				{"targetPoints", p.param<int>("preprocess", "random_downsample_target", 0)},
				{"kNeighbors", p.param<int>("preprocess", "k_correspondences", 8)},
				{"threads", p.param<int>("preprocess", "num_threads", 2)}}}};
		const auto o = config("config_odometry");
		const auto odometryLibrary = o.param<string>("odometry_estimation", "so_name", "");
		if (odometryLibrary == "libodometry_estimation_ct.so")
			result["odometry"] = {
				{"voxelResolution", o.param<double>("odometry_estimation", "ivox_resolution", 1.0)},
				{"iterations", o.param<int>("odometry_estimation", "lm_max_iterations", 10)},
				{"threads", o.param<int>("odometry_estimation", "num_threads", 2)}};
		else if (odometryLibrary == "libodometry_estimation_cpu.so")
			result["odometry"] = {
				{"voxelResolution", o.param<double>("odometry_estimation",
					o.param<string>("odometry_estimation", "registration_type", "VGICP") == "GICP" ? "ivox_resolution" : "vgicp_resolution", .5)},
				{"iterations", o.param<int>("odometry_estimation", "max_iterations", 5)},
				{"threads", o.param<int>("odometry_estimation", "num_threads", 2)}};
		const auto s = config("config_sub_mapping");
		if (s.param<string>("sub_mapping", "so_name", "") == "libsub_mapping.so")
			result["submap"] = {
				{"keyframes", s.param<int>("sub_mapping", "max_num_keyframes", 15)},
				{"keyframeStrategy", s.param<string>("sub_mapping", "keyframe_update_strategy", "OVERLAP")},
				{"keyframeTranslation", s.param<double>("sub_mapping", "keyframe_update_interval_trans", 1.0)},
				{"keyframeRotation", s.param<double>("sub_mapping", "keyframe_update_interval_rot", 3.15)},
				{"maxOverlap", s.param<double>("sub_mapping", "max_keyframe_overlap", .8)},
				{"voxelResolution", s.param<double>("sub_mapping", "submap_downsample_resolution", .25)}};
		const auto g = config("config_global_mapping");
		if (g.param<string>("global_mapping", "so_name", "") == "libglobal_mapping.so")
			result["global"] = {
				{"voxelResolution", g.param<double>("global_mapping", "submap_voxel_resolution", 1.0)},
				{"loopDistance", g.param<double>("global_mapping", "max_implicit_loop_distance", 100.0)},
				{"loopOverlap", g.param<double>("global_mapping", "min_implicit_loop_overlap", .1)}};
		return result;
	}

	json _GLIM::validatedParameters(const json &values) const
	{
		if (!values.is_object()) throw std::invalid_argument("config must be an object");
		json result = m_parameters;
		for (const auto &field : values.items())
		{
			if (!result.contains(field.key())) throw std::invalid_argument("Unknown GLIM parameter: " + field.key());
			if (!result[field.key()].is_object()) result[field.key()] = field.value();
			else
			{
				if (!field.value().is_object()) throw std::invalid_argument(field.key() + " must be an object");
				for (const auto &item : field.value().items())
				{
					if (!result[field.key()].contains(item.key()))
						throw std::invalid_argument("Unknown GLIM parameter: " + field.key() + "." + item.key());
					result[field.key()][item.key()] = item.value();
				}
			}
		}
		auto range = [&](const json &v, const string &name, double low, double high, bool integer = false) {
			if (!v.is_number() || (integer && !v.is_number_integer()) ||
				!std::isfinite(v.get<double>()) || v.get<double>() < low || v.get<double>() > high)
				throw std::invalid_argument(name + " must be " + (integer ? "an integer" : "a number") +
					" between " + std::to_string(low) + " and " + std::to_string(high));
		};
		if (!result["bMapping"].is_boolean()) throw std::invalid_argument("bMapping must be a boolean");
		range(result["nMinPoints"], "nMinPoints", 10, 1000000, true);
		const auto &p = result["preprocess"];
		range(p["distanceNear"], "preprocess.distanceNear", 0, 100000);
		range(p["distanceFar"], "preprocess.distanceFar", .001, 100000);
		if (p["distanceFar"].get<double>() <= p["distanceNear"].get<double>())
			throw std::invalid_argument("distanceFar must exceed distanceNear");
		range(p["voxelResolution"], "preprocess.voxelResolution", .001, 1000);
		range(p["targetPoints"], "preprocess.targetPoints", 0, 1000000, true);
		range(p["kNeighbors"], "preprocess.kNeighbors", 3, 1000, true);
		range(p["threads"], "preprocess.threads", 1, 128, true);
		if (result["nMinPoints"].get<int>() <= p["kNeighbors"].get<int>())
			throw std::invalid_argument("nMinPoints must exceed kNeighbors");
		if (p["targetPoints"].get<int>() && p["targetPoints"].get<int>() < result["nMinPoints"].get<int>())
			throw std::invalid_argument("targetPoints must be zero (automatic) or at least nMinPoints");
		if (result.contains("odometry"))
		{
			const auto &o = result["odometry"];
			range(o["voxelResolution"], "odometry.voxelResolution", .001, 1000);
			range(o["iterations"], "odometry.iterations", 1, 1000, true);
			range(o["threads"], "odometry.threads", 1, 128, true);
		}
		if (result.contains("submap"))
		{
			const auto &s = result["submap"];
			range(s["keyframes"], "submap.keyframes", 2, 1000, true);
			if (s["keyframeStrategy"] != "OVERLAP" && s["keyframeStrategy"] != "DISPLACEMENT")
				throw std::invalid_argument("keyframeStrategy must be OVERLAP or DISPLACEMENT");
			range(s["keyframeTranslation"], "submap.keyframeTranslation", 0, 10000);
			range(s["keyframeRotation"], "submap.keyframeRotation", 0, 2 * M_PI);
			range(s["maxOverlap"], "submap.maxOverlap", 0, 1);
			range(s["voxelResolution"], "submap.voxelResolution", .001, 1000);
		}
		if (result.contains("global"))
		{
			const auto &g = result["global"];
			range(g["voxelResolution"], "global.voxelResolution", .001, 1000);
			range(g["loopDistance"], "global.loopDistance", 0, 100000);
			range(g["loopOverlap"], "global.loopOverlap", 0, 1);
		}
		return result;
	}

	void _GLIM::applyParameters(const json &values)
	{
		m_parameters = values;
		m_bMapping = values["bMapping"].get<bool>();
		m_nMinPoints = values["nMinPoints"].get<int>();
	}

	bool _GLIM::saveConfig(void)
	{
		auto lock = lockSLAM();
		if (m_bTracking)
		{
			LOG_E("Stop SLAM before saving parameters");
			return false;
		}

		if (!_SLAMbase::saveConfig())
		{
			return false;
		}

		(*m_pJ)["parameters"] = m_parameters;

		return m_pJcfg->saveToFile();
	}

	bool _GLIM::startSLAM(void)
	{
		resetSLAM();
		std::lock_guard<std::mutex> lock(configMutex);
		const string path = std::filesystem::canonical(m_configPath).string();
		IF_Le_F(!std::filesystem::is_regular_file(path + "/config.json"), "Missing GLIM config.json: " + path);
		IF_Le_F(!configuredPath.empty() && configuredPath != path,
			"All GLIM modules must use the same configPath in one process");
		if (configuredPath.empty())
		{
			glim::GlobalConfig::instance(path, true);
			configuredPath = path;
		}

		readConfig("config_sensors");
		readConfig("config_preprocess");
		const auto odomConfig = readConfig("config_odometry");
		const auto odomLibrary = odomConfig.param<string>("odometry_estimation", "so_name", "");
		IF_Le_F(odomLibrary.empty(), "Missing GLIM odometry so_name");
		if (odomLibrary == "libodometry_estimation_ct.so")
		{
			glim::OdometryEstimationCTParams p;
			const auto &v = m_parameters["odometry"];
			p.ivox_resolution = v["voxelResolution"].get<double>();
			p.lm_max_iterations = v["iterations"].get<int>();
			p.num_threads = v["threads"].get<int>();
			m_odometry = std::make_shared<glim::OdometryEstimationCT>(p);
		}
		else if (odomLibrary == "libodometry_estimation_cpu.so")
		{
			glim::OdometryEstimationCPUParams p;
			const auto &v = m_parameters["odometry"];
			p.ivox_resolution = p.vgicp_resolution = v["voxelResolution"].get<double>();
			p.max_iterations = v["iterations"].get<int>();
			p.num_threads = v["threads"].get<int>();
			m_odometry = std::make_shared<glim::OdometryEstimationCPU>(p);
		}
		else m_odometry = glim::OdometryEstimationBase::load_module(odomLibrary);
		IF_Le_F(!m_odometry, "Cannot load GLIM odometry: " + odomLibrary);
		m_bRequiresIMU = m_odometry->requires_imu();
		IF_Le_F(m_bRequiresIMU && !m_pIMU, "Selected GLIM odometry requires _IMUbase");

		glim::CloudPreprocessorParams params;
		const auto &pre = m_parameters["preprocess"];
		params.distance_near_thresh = pre["distanceNear"].get<double>();
		params.distance_far_thresh = pre["distanceFar"].get<double>();
		params.downsample_resolution = pre["voxelResolution"].get<double>();
		params.downsample_target = pre["targetPoints"].get<int>();
		params.k_correspondences = pre["kNeighbors"].get<int>();
		params.num_threads = pre["threads"].get<int>();
		// _PointCloud contains complete depth frames, without per-point times.
		params.global_shutter = true;
		m_nMinPoints = std::max(m_nMinPoints, params.k_correspondences + 1);
		m_distanceNear = params.distance_near_thresh;
		m_distanceFar = params.distance_far_thresh;
		m_preprocessor = std::make_unique<glim::CloudPreprocessor>(params);

		if (m_bMapping)
		{
			const auto subConfig = readConfig("config_sub_mapping");
			const auto globalConfig = readConfig("config_global_mapping");
			IF_Le_F(!m_pIMU && (subConfig.param<bool>("sub_mapping", "enable_imu", true) ||
				globalConfig.param<bool>("global_mapping", "enable_imu", true)),
				"GLIM mapping with enable_imu requires _IMUbase");
			const auto subLibrary = subConfig.param<string>("sub_mapping", "so_name", "");
			const auto globalLibrary = globalConfig.param<string>("global_mapping", "so_name", "");
			IF_Le_F(subLibrary.empty() || globalLibrary.empty(), "Missing GLIM mapping so_name");
			if (m_parameters.contains("submap"))
			{
				glim::SubMappingParams p;
				const auto &v = m_parameters["submap"];
				p.max_num_keyframes = v["keyframes"].get<int>();
				p.keyframe_update_strategy = v["keyframeStrategy"].get<string>();
				p.keyframe_update_interval_trans = v["keyframeTranslation"].get<double>();
				p.keyframe_update_interval_rot = v["keyframeRotation"].get<double>();
				p.max_keyframe_overlap = v["maxOverlap"].get<double>();
				p.submap_downsample_resolution = v["voxelResolution"].get<double>();
				if (p.enable_imu) m_subMapping = std::make_shared<glim::SubMapping>(p);
				else m_subMapping = std::make_shared<IMUFreeSubMapping>(p);
			}
			else m_subMapping = glim::SubMappingBase::load_module(subLibrary);
			if (m_parameters.contains("global"))
			{
				glim::GlobalMappingParams p;
				const auto &v = m_parameters["global"];
				p.submap_voxel_resolution = v["voxelResolution"].get<double>();
				p.submap_voxel_resolution_max = globalConfig.has_param("global_mapping", "submap_voxel_resolution_max") ?
					std::max(p.submap_voxel_resolution, p.submap_voxel_resolution_max) : p.submap_voxel_resolution;
				p.max_implicit_loop_distance = v["loopDistance"].get<double>();
				p.min_implicit_loop_overlap = v["loopOverlap"].get<double>();
				m_globalMapping = std::make_shared<glim::GlobalMapping>(p);
			}
			else m_globalMapping = glim::GlobalMappingBase::load_module(globalLibrary);
			IF_Le_F(!m_subMapping || !m_globalMapping, "Cannot load GLIM mapping modules");
		}
		return true;
	}

	void _GLIM::updateSLAM(void)
	{
		const uint64_t started = getTns();
		uint64_t checkpoint = started;
		auto recordStage = [&](size_t stage) {
			const uint64_t now = getTns();
			m_stageTimeNs[stage] += now - checkpoint;
			checkpoint = now;
		};
		++m_updates;
		Vector3d acc, gyro;
		uint64_t stamp = 0;
		uint64_t previousIMU = m_tStampLastIMU;
		while (readIMU(acc, gyro, stamp))
		{
			if (previousIMU) m_maxIMUgapNs = std::max(m_maxIMUgapNs, stamp - previousIMU);
			previousIMU = stamp;
			++m_imuSamples;
			const double seconds = nsec2sec<double>(stamp);
			m_odometry->insert_imu(seconds, acc, gyro);
			if (m_subMapping)
				m_subMapping->insert_imu(seconds, acc, gyro);
			if (m_globalMapping)
				m_globalMapping->insert_imu(seconds, acc, gyro);
		}
		recordStage(0);

		const uint64_t previousFrame = m_tStampLastFrame;
		if (readPointCloud(m_inputBuffer, stamp))
		{
			if (previousFrame) m_frameIntervalMs = (stamp - previousFrame) / double(NSEC_MSEC);
			auto raw = std::make_shared<glim::RawPoints>();
			raw->stamp = nsec2sec<double>(stamp);
			raw->points.reserve(m_inputBuffer.size());
			const double nearSquared = m_distanceNear * m_distanceNear;
			const double farSquared = m_distanceFar * m_distanceFar;
			for (const auto &point : m_inputBuffer)
			{
				const float *p = point.data();
				IF_CONT(!std::isfinite(p[0]) || !std::isfinite(p[1]) || !std::isfinite(p[2]));
				const double distanceSquared = double(p[0]) * p[0] + double(p[1]) * p[1] + double(p[2]) * p[2];
				IF_CONT(distanceSquared <= nearSquared || distanceSquared >= farSquared);
				raw->points.emplace_back(p[0], p[1], p[2], 1.0);
			}
			raw->times.assign(raw->points.size(), 0.0);
			m_pendingFrame = raw;
		}
		recordStage(1);

		IF_(!m_pendingFrame);
		// Wait for IMU coverage; keep at most the newest depth frame when lagging.
		IF_(m_bRequiresIMU && nsec2sec<double>(m_tStampLastIMU) <= m_pendingFrame->stamp);
		auto raw = std::move(m_pendingFrame);
		m_inputPoints = raw->size();
		if (raw->size() < m_nMinPoints)
		{
			setConfidence(0.0f);
			return;
		}
		auto frame = m_preprocessor->preprocess(raw);
		recordStage(2);
		m_registrationPoints = frame ? frame->size() : 0;
		if (!frame || frame->size() < m_nMinPoints)
		{
			setConfidence(0.0f);
			return;
		}
		frame->raw_points = raw;
		vector<glim::EstimationFrame::ConstPtr> marginalized;
		auto result = m_odometry->insert_frame(frame, marginalized);
		recordStage(3);
		++m_processedFrames;
		if (result)
		{
			if (!m_bRequiresIMU) m_activeFrames[result->id] = result;
			// GLIM has no scalar tracking-quality score: report fresh pose availability.
			if (publishPose(result->T_world_lidar, 100.0f))
			{
				m_latestFrame = result->clone_wo_points();
				// Retain a bounded preview for on-demand PLY export, even when no
				// legacy global-map output is connected. It is never streamed.
				if (result->frame)
				{
					// Retain processed points only, not full-resolution raw clouds,
					// voxel maps or backend-specific state for every preview frame.
					auto preview = result->clone_wo_points();
					preview->frame = result->frame;
					m_liveFrames.push_back({preview, result});
				}
				while (m_liveFrames.size() > size_t(m_nLiveFrames)) m_liveFrames.pop_front();
			}
		}
		else
			setConfidence(0.0f);
		insertMappingFrames(marginalized);
		collectSubmaps();
		recordStage(4);
		publishMap();
		recordStage(5);
		m_processingMs = (getTns() - started) / double(NSEC_MSEC);
	}

	void _GLIM::insertMappingFrames(const vector<glim::EstimationFrame::ConstPtr> &frames)
	{
		for (const auto &frame : frames)
		{
			IF_CONT(!frame);
			m_activeFrames.erase(frame->id);
			// Replace preview poses with the smoother's final marginalized poses.
			for (auto &live : m_liveFrames) if (live.preview->id == frame->id)
			{
				auto refined = frame->clone_wo_points();
				refined->frame = frame->frame ? frame->frame : live.preview->frame;
				live.preview = refined;
				live.source.reset();
				break;
			}
			if (m_subMapping) m_subMapping->insert_frame(frame);
		}
	}

	void _GLIM::collectSubmaps(void)
	{
		IF_(!m_subMapping || !m_globalMapping);
		for (const auto &submap : m_subMapping->get_submaps())
			insertSubmap(submap);
	}

	void _GLIM::insertSubmap(const std::shared_ptr<glim::SubMap> &submap)
	{
		IF_(!submap);
		m_globalMapping->insert_submap(submap);
		// The synchronous mapping backend updates these shared submap poses on
		// this same worker, under m_mtxSLAM. No process-global callbacks needed.
		m_submaps.push_back(submap);
		auto points = std::make_shared<vector<Vector3f>>();
		if (submap->frame && submap->frame->points)
		{
			points->reserve(submap->frame->size());
			for (size_t i = 0; i < submap->frame->size(); ++i)
			{
				const Vector3f p = submap->frame->points[i].head<3>().cast<float>();
				if (p.allFinite()) points->push_back(p);
			}
		}
		GLIM_SUBMAP web;
		web.id = m_webSubmaps.size();
		web.timestampNs = submap->odom_frames.empty() ? 0 : sec2nsec(submap->odom_frames.back()->stamp);
		web.points = points;
		m_submapPoints += points->size();
		m_webSubmaps.push_back(std::move(web));
		refreshSubmapPoses();
		if (!submap->odom_frames.empty())
			m_submapStamp = std::max(m_submapStamp, submap->odom_frames.back()->stamp);
		while (!m_liveFrames.empty() && m_liveFrames.front().preview->stamp <= m_submapStamp)
			m_liveFrames.pop_front();
	}

	void _GLIM::refreshSubmapPoses()
	{
		for (size_t i = 0; i < m_webSubmaps.size(); ++i)
			m_webSubmaps[i].pose = m_submaps[i]->T_world_origin;
		++m_revision;
		m_mapDirty = true;
	}

	GLIM_MAP_SNAPSHOT _GLIM::submapSnapshot(uint64_t knownSession, uint64_t knownRevision)
	{
		auto lock = lockSLAM();
		GLIM_MAP_SNAPSHOT result;
		result.session = m_session; result.revision = m_revision;
		if (knownSession != m_session || knownRevision != m_revision)
			result.submaps = m_webSubmaps;
		return result;
	}

	Isometry3d _GLIM::mapCorrection() const
	{
		if (m_submaps.empty() || m_submaps.back()->odom_frames.empty()) return Isometry3d::Identity();
		const auto &submap = m_submaps.back();
		return submap->T_world_origin * submap->origin_odom_frame()->T_world_sensor().inverse();
	}

	void _GLIM::publishMap(bool force)
	{
		IF_(!m_pGlobalMap);
		IF_(!force && !m_bPublishLiveMap && !m_mapDirty);
		const uint64_t now = getTns();
		IF_(!force && m_mapUpdatedNs && now - m_mapUpdatedNs < m_mapIntervalNs);
		collectMapPoints(m_mapBuffer, m_mapColors, size_t(m_nMapPoints), m_bPublishLiveMap);
		m_pGlobalMap->setFrame(m_mapBuffer, m_mapColors, now);
		m_mapPoints = m_mapBuffer.size();
		m_mapUpdatedNs = now;
		m_mapDirty = false;
	}

	void _GLIM::collectMapPoints(vector<Vector3f> &points, vector<Vector3f> &colors,
		size_t limit, bool includeLive)
	{
		size_t total = 0;
		for (const auto &submap : m_submaps) if (submap->frame) total += submap->frame->size();
		if (includeLive)
			for (const auto &live : m_liveFrames) if (live.preview->frame) total += live.preview->frame->size();
		const size_t stride = limit ? std::max(size_t(1), (total + limit - 1) / limit) : 1;
		points.clear(); colors.clear();
		points.reserve(limit ? std::min(total, limit) : total);
		colors.reserve(points.capacity());
		size_t offset = 0;
		auto append = [&](const gtsam_points::PointCloud::ConstPtr &cloud, const Isometry3d &pose, const Vector3f &color)
		{
			if (!cloud || !cloud->points) return;
			// Cache the column-major affine coefficients once per cloud. Expanding
			// this small kernel avoids an Eigen expression/evaluator per map point
			// in Debug builds, while keeping the original double-precision transform.
			const double *m = pose.data();
			for (size_t i = (stride - offset % stride) % stride; i < cloud->size(); i += stride)
			{
				const double *p = cloud->points[i].data();
				const float x = m[0] * p[0] + m[4] * p[1] + m[8] * p[2] + m[12] * p[3];
				const float y = m[1] * p[0] + m[5] * p[1] + m[9] * p[2] + m[13] * p[3];
				const float z = m[2] * p[0] + m[6] * p[1] + m[10] * p[2] + m[14] * p[3];
				if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) continue;
				const Vector3f point(x, y, z);
				points.push_back(point); colors.push_back(color);
			}
			offset += cloud->size();
		};
		for (const auto &submap : m_submaps)
			append(submap->frame, submap->T_world_origin, Vector3f(0.65f, 0.82f, 0.95f));
		if (!includeLive) return;
		const Isometry3d correction = mapCorrection();
		for (auto &live : m_liveFrames)
		{
			// The fixed-lag smoother revises active poses on every insertion.
			// Follow those revisions immediately without retaining backend raw clouds.
			if (auto source = live.source.lock())
			{
				live.preview->T_world_imu = source->T_world_imu;
				live.preview->T_world_lidar = source->T_world_lidar;
				live.preview->frame_id = source->frame_id;
			}
			append(live.preview->frame, correction * live.preview->T_world_sensor(), Vector3f(0.3f, 0.95f, 0.7f));
		}
	}

	void _GLIM::stopSLAM(void)
	{
		if (m_odometry)
			insertMappingFrames(m_odometry->get_remaining_frames());
		// CT inherits an empty get_remaining_frames(). Its final window still
		// belongs in the map, even for scans shorter than the smoother lag.
		// Frames returned above were erased, so this also avoids duplicate input.
		vector<glim::EstimationFrame::ConstPtr> remaining;
		remaining.reserve(m_activeFrames.size());
		for (const auto &entry : m_activeFrames) remaining.push_back(entry.second);
		insertMappingFrames(remaining);
		collectSubmaps();
		if (m_subMapping && m_globalMapping)
		{
			for (const auto &submap : m_subMapping->submit_end_of_sequence())
				insertSubmap(submap);
			m_globalMapping->optimize();
			refreshSubmapPoses();
		}
		publishMap(true);
		m_pendingFrame.reset();
		m_odometry.reset();
		m_subMapping.reset();
		m_preprocessor.reset();
	}

	void _GLIM::resetSLAM(void)
	{
		if (m_pGlobalMap) m_pGlobalMap->clear();
		m_submaps.clear(); m_liveFrames.clear(); m_activeFrames.clear(); m_latestFrame.reset();
		m_webSubmaps.clear(); m_submapPoints = 0;
		++m_session; m_revision = 0; m_mapDirty = false;
		m_mapUpdatedNs = m_mapPoints = m_processedFrames = 0;
		m_imuSamples = m_maxIMUgapNs = 0;
		m_frameIntervalMs = m_processingMs = 0.0;
		m_stageTimeNs.fill(0);
		m_updates = m_inputPoints = m_registrationPoints = 0;
		m_inputBuffer.clear(); m_mapBuffer.clear(); m_mapColors.clear();
		m_submapStamp = -1.0;
		setPos(Vector3d::Zero());
		setOrientation(Quaterniond::Identity(), true);
		m_pendingFrame.reset();
		m_odometry.reset();
		m_subMapping.reset();
		m_globalMapping.reset();
		m_preprocessor.reset();
	}

	bool _GLIM::saveMap(const string &path)
	{
		auto lock = lockSLAM();
		IF_Le_F(m_bTracking || !m_globalMapping || path.empty(), "Stop a GLIM mapping session before saveMap");
		try
		{
			std::filesystem::create_directories(path);
			m_globalMapping->save(path);
			return true;
		}
		catch (const std::exception &e)
		{
			LOG_E(string("Cannot save GLIM map: ") + e.what());
			return false;
		}
	}

	bool _GLIM::savePointCloud(const string &path, size_t &count, string &error)
	{
		count = 0;
		if (std::filesystem::path(path).extension() != ".ply")
		{
			error = "Point-cloud filename must end in .ply";
			return false;
		}
		vector<Vector3f> points, colors;
		{
			auto lock = lockSLAM();
			// Snapshot all completed submaps and the retained unfinished preview.
			// File IO happens after releasing the estimator's lock.
			collectMapPoints(points, colors, 0, true);
		}
		if (points.empty()) { error = "No map points are available to save"; return false; }
		if (!_PCfile::savePLY(path, points, colors, &error)) return false;
		count = points.size();
		return true;
	}

	json _GLIM::status(void)
	{
		auto lock = lockSLAM();
		const float quality = confidence();
		const Isometry3d pose = mapCorrection() * m_mPose;
		const Quaterniond q(pose.linear());
		const auto &r = pose.linear();
		const Vector3d angles(std::atan2(r(2, 1), r(2, 2)),
			std::atan2(-r(2, 0), std::hypot(r(2, 1), r(2, 2))), std::atan2(r(1, 0), r(0, 0)));
		return {{"state", !m_slamError.empty() ? "error" : !m_bTracking ? "stopped" : quality > 0 ? "tracking" : m_latestFrame ? "waiting" : "initializing"},
			{"tracking", m_bTracking.load()}, {"poseValid", bool(m_latestFrame)}, {"poseFresh", quality > 0},
			{"confidence", quality}, {"error", m_slamError}, {"mapping", m_bMapping},
			{"position", {pose.translation().x(), pose.translation().y(), pose.translation().z()}},
			{"orientation", {q.x(), q.y(), q.z(), q.w()}},
			{"angles", {angles.x() * 180 / M_PI, angles.y() * 180 / M_PI, angles.z() * 180 / M_PI}},
			{"poseTimestampNs", std::to_string(m_latestFrame ? sec2nsec(m_latestFrame->stamp) : 0)},
			{"inputTimestampNs", std::to_string(m_tStampLastFrame)}, {"frames", m_processedFrames},
			{"imuSamples", m_imuSamples}, {"maxIMUgapNs", m_maxIMUgapNs},
			{"frameIntervalMs", m_frameIntervalMs}, {"processingMs", m_processingMs},
			{"updates", m_updates}, {"inputPoints", m_inputPoints}, {"registrationPoints", m_registrationPoints},
			{"workMs", {{"imu", m_stageTimeNs[0] / double(NSEC_MSEC)}, {"input", m_stageTimeNs[1] / double(NSEC_MSEC)},
				{"preprocess", m_stageTimeNs[2] / double(NSEC_MSEC)}, {"odometry", m_stageTimeNs[3] / double(NSEC_MSEC)},
				{"mapping", m_stageTimeNs[4] / double(NSEC_MSEC)}, {"publishMap", m_stageTimeNs[5] / double(NSEC_MSEC)}}},
			{"session", std::to_string(m_session)}, {"revision", std::to_string(m_revision)},
			{"canSavePointCloud", !m_submaps.empty() || !m_liveFrames.empty()},
			{"mapPoints", m_bPublishLiveMap ? m_mapPoints : m_submapPoints}, {"submaps", m_submaps.size()}, {"liveFrames", m_liveFrames.size()},
			{"mapTimestampNs", std::to_string(m_mapUpdatedNs)}, {"mapOutput", m_pGlobalMap ? m_pGlobalMap->getName() : ""},
			{"frame", "map"}};
	}

	void _GLIM::console(const json &j, void *pJSONbase)
	{
		auto *transport = static_cast<_JSONbase *>(pJSONbase);
		if (!transport || !j.is_object() || !j.contains("cmd") || !j["cmd"].is_string()) return;
		const string cmd = j["cmd"].get<string>();
		json reply = {{"cmd", cmd}, {"module", getName()}, {"bSuccess", true}};
		if (j.contains("requestId")) reply["requestId"] = j["requestId"];
		try
		{
			if (cmd == "start") reply["bSuccess"] = startTracking();
			else if (cmd == "stop") stopTracking();
			else if (cmd == "reset") reset();
			else if (cmd == "setConfig")
			{
				auto lock = lockSLAM();
				if (m_bTracking) throw std::runtime_error("Stop SLAM before changing parameters");
				applyParameters(validatedParameters(j.at("config")));
			}
			else if (cmd == "saveConfig")
			{
				reply["bSuccess"] = saveConfig();
				if (!reply["bSuccess"].get<bool>())
				{
					reply["error"] = "Could not save the launch configuration";
				}
			}
			else if (cmd == "savePointCloud")
			{
				string path = j.value("path", string());
				if (path.empty())
				{
					std::filesystem::create_directories(m_exportPath);
					path = (std::filesystem::path(m_exportPath) / ("map-" + std::to_string(getTns()) + ".ply")).string();
				}
				size_t points = 0;
				string error;
				reply["bSuccess"] = savePointCloud(path, points, error);
				reply["path"] = std::filesystem::absolute(path).string();
				reply["points"] = points;
				if (!error.empty()) reply["error"] = error;
			}
			else if (cmd == "getConfig") {}
			else if (cmd != "getStatus")
			{
				reply["bSuccess"] = false;
				reply["error"] = "Unknown GLIM command: " + cmd;
			}
			if (cmd == "getConfig" || cmd == "setConfig" || cmd == "saveConfig")
			{
				auto lock = lockSLAM();
				reply["config"] = m_parameters;
				reply["configFile"] = m_pJcfg->getFileName();
			}
			reply["status"] = status();
			// A previous session failure belongs to status, not to a successful
			// getConfig/saveConfig request used to diagnose and fix that failure.
			if (cmd == "start" && !reply["status"]["error"].get<string>().empty())
			{
				reply["bSuccess"] = false;
				reply["error"] = reply["status"]["error"];
			}
		}
		catch (const std::exception &e) { reply["bSuccess"] = false; reply["error"] = e.what(); }
		transport->sendJson(reply);
	}

}
