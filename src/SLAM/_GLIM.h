/*
 * _GLIM.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__GLIM_H_
#define OpenKAI_src_SLAM__GLIM_H_

#include "_SLAMbase.h"
#include "../DataStream/PCLmap.h"
#include <array>
#include <deque>
#include <map>

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
	class _GLIM : public _SLAMbase
	{
	public:
		_GLIM();
		~_GLIM() override;
		bool loadConfig(void) override;
		bool link(InstanceMgr *pM) override;
		using _SLAMbase::console;
		void console(const json &j, void *pJSONbase) override;
		json status(void);
		bool saveConfig(bool bExport) override;
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
		void collectMapPoints(vector<GEOMETRY_POINT> &points, size_t limit, bool includeLive, uint64_t tStamp);

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
		PCLframe *m_pGlobalMap = nullptr;
		PCLmap *m_pSubmapStream = nullptr;
		vector<std::shared_ptr<glim::SubMap>> m_submaps;
		uint64_t m_session = 0, m_revision = 0;
		size_t m_submapPoints = 0;
		struct LiveFrame
		{
			std::shared_ptr<glim::EstimationFrame> preview;
			std::weak_ptr<const glim::EstimationFrame> source;
		};
		std::deque<LiveFrame> m_liveFrames;
		// IMU-free backends may not return their active smoothing window on Stop.
		// Keep the actual frames until mapping consumes them, independent of previews.
		std::map<long, std::shared_ptr<const glim::EstimationFrame>> m_activeFrames;
		std::shared_ptr<const glim::EstimationFrame> m_latestFrame;
		int m_nMapPoints = 400000, m_nLiveFrames = 100;
		uint64_t m_mapIntervalNs = 200 * NSEC_MSEC;
		uint64_t m_mapUpdatedNs = 0;
		size_t m_mapPoints = 0, m_processedFrames = 0;
		size_t m_imuSamples = 0;
		uint64_t m_maxIMUgapNs = 0;
		double m_frameIntervalMs = 0.0, m_processingMs = 0.0;
		// Cumulative work time, including polls that do not produce a pose.
		std::array<uint64_t, 6> m_stageTimeNs{};
		size_t m_updates = 0, m_inputPoints = 0, m_registrationPoints = 0;
		double m_submapStamp = -1.0;
	};
}
#endif
