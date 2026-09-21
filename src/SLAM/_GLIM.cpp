/*
 * _GLIM.cpp
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#include "_GLIM.h"

#include <filesystem>
#include <glim/util/config.hpp>
#include <glim/preprocess/cloud_preprocessor.hpp>
#include <glim/odometry/odometry_estimation_base.hpp>
#include <glim/mapping/sub_mapping_base.hpp>
#include <glim/mapping/global_mapping_base.hpp>

namespace kai
{
	namespace
	{
		// GLIM's configuration is process-wide. Never replace it beneath a session.
		std::mutex configMutex;
		string configuredPath;

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

	bool _GLIM::init(const json &j)
	{
		IF_F(!_SLAMbase::init(j));
		jKv(j, "configPath", m_configPath);
		jKv(j, "bMapping", m_bMapping);
		jKv(j, "nMinPoints", m_nMinPoints);
		jKv(j, "nMapPoints", m_nMapPoints);
		jKv(j, "nLiveFrames", m_nLiveFrames);
		jKv(j, "tMapUpdateUs", m_mapIntervalUs);
		IF_Le_F(m_configPath.empty(), "GLIM configPath is required");
		IF_Le_F(m_nMinPoints < 10, "GLIM nMinPoints must be at least 10");
		IF_Le_F(m_nMapPoints < 1 || m_nLiveFrames < 1, "GLIM map limits must be positive");
		return true;
	}

	bool _GLIM::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!_SLAMbase::link(j, pM));
		string name;
		jKv(j, "globalMapPCL", name);
		m_pGlobalMap = name.empty() ? nullptr : dynamic_cast<_PointCloud *>(static_cast<BASE *>(pM->findModule(name)));
		IF_Le_F(!name.empty() && !m_pGlobalMap, "Cannot find globalMapPCL: " + name);
		IF_Le_F(m_pGlobalMap && m_pGlobalMap == m_pPCL, "GLIM input and globalMapPCL must be different buffers");
		return true;
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
		m_odometry = glim::OdometryEstimationBase::load_module(odomLibrary);
		IF_Le_F(!m_odometry, "Cannot load GLIM odometry: " + odomLibrary);
		m_bRequiresIMU = m_odometry->requires_imu();
		IF_Le_F(m_bRequiresIMU && !m_pIMU, "Selected GLIM odometry requires _IMUbase");

		glim::CloudPreprocessorParams params;
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
			m_subMapping = glim::SubMappingBase::load_module(subLibrary);
			m_globalMapping = glim::GlobalMappingBase::load_module(globalLibrary);
			IF_Le_F(!m_subMapping || !m_globalMapping, "Cannot load GLIM mapping modules");
		}
		return true;
	}

	void _GLIM::updateSLAM(void)
	{
		const uint64_t started = getTbootUs();
		Vector3d acc, gyro;
		uint64_t stamp = 0;
		uint64_t previousIMU = m_tStampLastIMU;
		while (readIMU(acc, gyro, stamp))
		{
			if (previousIMU) m_maxIMUgapUs = std::max(m_maxIMUgapUs, stamp - previousIMU);
			previousIMU = stamp;
			++m_imuSamples;
			const double seconds = usec2sec<double>(stamp);
			m_odometry->insert_imu(seconds, acc, gyro);
			if (m_subMapping)
				m_subMapping->insert_imu(seconds, acc, gyro);
			if (m_globalMapping)
				m_globalMapping->insert_imu(seconds, acc, gyro);
		}

		vector<Vector3f> points;
		const uint64_t previousFrame = m_tStampLastFrame;
		if (readPointCloud(points, stamp))
		{
			if (previousFrame) m_frameIntervalMs = (stamp - previousFrame) * 0.001;
			auto raw = std::make_shared<glim::RawPoints>();
			raw->stamp = usec2sec<double>(stamp);
			raw->points.reserve(points.size());
			for (const auto &point : points)
			{
				IF_CONT(!point.allFinite());
				const double distanceSquared = point.squaredNorm();
				IF_CONT(distanceSquared <= m_distanceNear * m_distanceNear || distanceSquared >= m_distanceFar * m_distanceFar);
				raw->points.emplace_back(point.x(), point.y(), point.z(), 1.0);
			}
			raw->times.assign(raw->points.size(), 0.0);
			m_pendingFrame = raw;
		}

		IF_(!m_pendingFrame);
		// Wait for IMU coverage; keep at most the newest depth frame when lagging.
		IF_(m_bRequiresIMU && usec2sec<double>(m_tStampLastIMU) <= m_pendingFrame->stamp);
		auto raw = std::move(m_pendingFrame);
		if (raw->size() < m_nMinPoints)
		{
			setConfidence(0.0f);
			return;
		}
		auto frame = m_preprocessor->preprocess(raw);
		if (!frame || frame->size() < m_nMinPoints)
		{
			setConfidence(0.0f);
			return;
		}
		frame->raw_points = raw;
		vector<glim::EstimationFrame::ConstPtr> marginalized;
		auto result = m_odometry->insert_frame(frame, marginalized);
		++m_processedFrames;
		if (result)
		{
			// GLIM has no scalar tracking-quality score: report fresh pose availability.
			if (publishPose(result->T_world_lidar, 100.0f))
			{
				m_latestFrame = result->clone_wo_points();
				if (m_pGlobalMap)
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
		publishMap();
		m_processingMs = (getTbootUs() - started) * 0.001;
	}

	void _GLIM::insertMappingFrames(const vector<glim::EstimationFrame::ConstPtr> &frames)
	{
		for (const auto &frame : frames)
		{
			IF_CONT(!frame);
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
		if (!submap->odom_frames.empty())
			m_submapStamp = std::max(m_submapStamp, submap->odom_frames.back()->stamp);
		while (!m_liveFrames.empty() && m_liveFrames.front().preview->stamp <= m_submapStamp)
			m_liveFrames.pop_front();
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
		const uint64_t now = getTbootUs();
		IF_(!force && m_mapUpdatedUs && now - m_mapUpdatedUs < m_mapIntervalUs);
		size_t total = 0;
		for (const auto &submap : m_submaps) if (submap->frame) total += submap->frame->size();
		for (const auto &live : m_liveFrames) if (live.preview->frame) total += live.preview->frame->size();
		const size_t stride = std::max(size_t(1), (total + m_nMapPoints - 1) / m_nMapPoints);
		vector<Vector3f> points, colors;
		points.reserve(std::min(total, size_t(m_nMapPoints)));
		colors.reserve(points.capacity());
		size_t offset = 0;
		auto append = [&](const gtsam_points::PointCloud::ConstPtr &cloud, const Isometry3d &pose, const Vector3f &color)
		{
			if (!cloud || !cloud->points) return;
			for (size_t i = (stride - offset % stride) % stride; i < cloud->size(); i += stride)
			{
				const Vector3f point = (pose * cloud->points[i]).head<3>().cast<float>();
				if (!point.allFinite()) continue;
				points.push_back(point); colors.push_back(color);
			}
			offset += cloud->size();
		};
		for (const auto &submap : m_submaps)
			append(submap->frame, submap->T_world_origin, Vector3f(0.65f, 0.82f, 0.95f));
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
		m_pGlobalMap->setFrame(points, colors, now);
		m_mapPoints = points.size();
		m_mapUpdatedUs = now;
	}

	void _GLIM::stopSLAM(void)
	{
		if (m_odometry)
			insertMappingFrames(m_odometry->get_remaining_frames());
		collectSubmaps();
		if (m_subMapping && m_globalMapping)
		{
			for (const auto &submap : m_subMapping->submit_end_of_sequence())
				insertSubmap(submap);
			m_globalMapping->optimize();
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
		m_submaps.clear(); m_liveFrames.clear(); m_latestFrame.reset();
		m_mapUpdatedUs = m_mapPoints = m_processedFrames = 0;
		m_imuSamples = m_maxIMUgapUs = 0;
		m_frameIntervalMs = m_processingMs = 0.0;
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
			{"poseTimestampUs", m_latestFrame ? uint64_t(m_latestFrame->stamp * 1e6) : 0},
			{"inputTimestampUs", m_tStampLastFrame}, {"frames", m_processedFrames},
			{"imuSamples", m_imuSamples}, {"maxIMUgapUs", m_maxIMUgapUs},
			{"frameIntervalMs", m_frameIntervalMs}, {"processingMs", m_processingMs},
			{"mapPoints", m_mapPoints}, {"submaps", m_submaps.size()}, {"liveFrames", m_liveFrames.size()},
			{"mapTimestampUs", m_mapUpdatedUs}, {"mapOutput", m_pGlobalMap ? m_pGlobalMap->getName() : ""},
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
			else if (cmd != "getStatus")
			{
				reply["bSuccess"] = false;
				reply["error"] = "Unknown GLIM command: " + cmd;
			}
			reply["status"] = status();
			if (!reply["status"]["error"].get<string>().empty())
			{
				reply["bSuccess"] = false;
				reply["error"] = reply["status"]["error"];
			}
		}
		catch (const std::exception &e) { reply["bSuccess"] = false; reply["error"] = e.what(); }
		transport->sendJson(reply);
	}

}
