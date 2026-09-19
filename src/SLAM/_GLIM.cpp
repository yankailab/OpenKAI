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
		IF_Le_F(m_configPath.empty(), "GLIM configPath is required");
		IF_Le_F(m_nMinPoints < 10, "GLIM nMinPoints must be at least 10");
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
		Vector3d acc, gyro;
		uint64_t stamp = 0;
		while (readIMU(acc, gyro, stamp))
		{
			const double seconds = usec2sec<double>(stamp);
			m_odometry->insert_imu(seconds, acc, gyro);
			if (m_subMapping)
				m_subMapping->insert_imu(seconds, acc, gyro);
			if (m_globalMapping)
				m_globalMapping->insert_imu(seconds, acc, gyro);
		}

		vector<Vector3f> points;
		if (readPointCloud(points, stamp))
		{
			auto raw = std::make_shared<glim::RawPoints>();
			raw->stamp = usec2sec<double>(stamp);
			raw->points.reserve(points.size());
			for (const auto &point : points)
			{
				IF_CONT(!point.allFinite());
				const double distance = point.cast<double>().norm();
				IF_CONT(distance <= m_distanceNear || distance >= m_distanceFar);
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
		if (result)
			// GLIM has no scalar tracking-quality score: report fresh pose availability.
			publishPose(result->T_world_lidar, 100.0f);
		else
			setConfidence(0.0f);
		insertMappingFrames(marginalized);
		collectSubmaps();
	}

	void _GLIM::insertMappingFrames(const vector<glim::EstimationFrame::ConstPtr> &frames)
	{
		IF_(!m_subMapping);
		for (const auto &frame : frames)
			if (frame)
				m_subMapping->insert_frame(frame);
	}

	void _GLIM::collectSubmaps(void)
	{
		IF_(!m_subMapping || !m_globalMapping);
		for (const auto &submap : m_subMapping->get_submaps())
			m_globalMapping->insert_submap(submap);
	}

	void _GLIM::stopSLAM(void)
	{
		if (m_odometry)
			insertMappingFrames(m_odometry->get_remaining_frames());
		collectSubmaps();
		if (m_subMapping && m_globalMapping)
		{
			for (const auto &submap : m_subMapping->submit_end_of_sequence())
				m_globalMapping->insert_submap(submap);
			m_globalMapping->optimize();
		}
		m_pendingFrame.reset();
		m_odometry.reset();
		m_subMapping.reset();
		m_preprocessor.reset();
	}

	void _GLIM::resetSLAM(void)
	{
		m_pendingFrame.reset();
		m_odometry.reset();
		m_subMapping.reset();
		m_globalMapping.reset();
		m_preprocessor.reset();
	}

	bool _GLIM::saveMap(const string &path)
	{
		std::lock_guard<std::mutex> lock(m_mtxSLAM);
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
}
