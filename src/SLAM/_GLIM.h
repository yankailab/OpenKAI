/*
 * _GLIM.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__GLIM_H_
#define OpenKAI_src_SLAM__GLIM_H_

#include "_SLAMbase.h"
#include <array>
#include <deque>

namespace glim
{
	class CloudPreprocessor;
	class OdometryEstimationBase;
	class SubMappingBase;
	class GlobalMappingBase;
	struct RawPoints;
	struct EstimationFrame;
	struct SubMap;
}

namespace kai
{
	// Immutable local submap points shared with viewers; poses may be corrected
	// when a later submap closes a loop. All snapshots are acquired under SLAM's lock.
	struct GLIM_SUBMAP
	{
		uint64_t id = 0, timestampUs = 0;
		Isometry3d pose = Isometry3d::Identity();
		std::shared_ptr<const vector<Vector3f>> points;
	};
	struct GLIM_MAP_SNAPSHOT
	{
		uint64_t session = 0, revision = 0;
		vector<GLIM_SUBMAP> submaps;
	};

	class _GLIM : public _SLAMbase
	{
	public:
		_GLIM();
		~_GLIM() override;
		bool init(const json &j) override;
		bool link(const json &j, ModuleMgr *pM) override;
		using _SLAMbase::console;
		void console(const json &j, void *pJSONbase) override;
		json status(void);
		GLIM_MAP_SNAPSHOT submapSnapshot(uint64_t knownSession, uint64_t knownRevision);
		bool loadConfig(json *pJ = nullptr, string fName = "") override;
		bool saveConfig(json &j, string fName = "") override;
		bool savePointCloud(const string &path, size_t &count, string &error);

		// Stop tracking first. The finished map survives stopTracking().
		bool saveMap(const string &path);

	protected:
		bool startSLAM(void) override;
		void stopSLAM(void) override;
		void resetSLAM(void) override;
		void updateSLAM(void) override;

	private:
		void insertMappingFrames(const vector<std::shared_ptr<const glim::EstimationFrame>> &frames);
		void collectSubmaps(void);
		void insertSubmap(const std::shared_ptr<glim::SubMap> &submap);
		void publishMap(bool force = false);
		Isometry3d mapCorrection() const;
		json parameterDefaults() const;
		json validatedParameters(const json &values) const;
		void applyParameters(const json &values);
		void refreshSubmapPoses();
		void collectMapPoints(vector<Vector3f> &points, vector<Vector3f> &colors,
			size_t limit, bool includeLive);

		string m_configPath;
		bool m_bMapping = true;
		bool m_bRequiresIMU = true;
		bool m_bPublishLiveMap = false, m_mapDirty = false;
		json m_parameters;
		string m_exportPath = "data/glim";
		int m_nMinPoints = 100;
		double m_distanceNear = 0.0;
		double m_distanceFar = 0.0;
		std::unique_ptr<glim::CloudPreprocessor> m_preprocessor;
		std::shared_ptr<glim::OdometryEstimationBase> m_odometry;
		std::shared_ptr<glim::SubMappingBase> m_subMapping;
		std::shared_ptr<glim::GlobalMappingBase> m_globalMapping;
		std::shared_ptr<glim::RawPoints> m_pendingFrame;
		vector<Vector3f> m_inputBuffer, m_mapBuffer, m_mapColors;
		_PointCloud *m_pGlobalMap = nullptr;
		vector<std::shared_ptr<glim::SubMap>> m_submaps;
		vector<GLIM_SUBMAP> m_webSubmaps;
		uint64_t m_session = 0, m_revision = 0;
		size_t m_submapPoints = 0;
		struct LiveFrame
		{
			std::shared_ptr<glim::EstimationFrame> preview;
			std::weak_ptr<const glim::EstimationFrame> source;
		};
		std::deque<LiveFrame> m_liveFrames;
		std::shared_ptr<const glim::EstimationFrame> m_latestFrame;
		int m_nMapPoints = 400000, m_nLiveFrames = 100;
		uint64_t m_mapIntervalUs = 200000, m_mapUpdatedUs = 0;
		size_t m_mapPoints = 0, m_processedFrames = 0;
		size_t m_imuSamples = 0;
		uint64_t m_maxIMUgapUs = 0;
		double m_frameIntervalMs = 0.0, m_processingMs = 0.0;
		// Cumulative work time, including polls that do not produce a pose.
		std::array<uint64_t, 6> m_stageTimeUs{};
		size_t m_updates = 0, m_inputPoints = 0, m_registrationPoints = 0;
		double m_submapStamp = -1.0;
	};
}
#endif
