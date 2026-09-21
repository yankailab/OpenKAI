/*
 * _GLIM.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__GLIM_H_
#define OpenKAI_src_SLAM__GLIM_H_

#include "_SLAMbase.h"
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

		string m_configPath;
		bool m_bMapping = true;
		bool m_bRequiresIMU = true;
		int m_nMinPoints = 100;
		double m_distanceNear = 0.0;
		double m_distanceFar = 0.0;
		std::unique_ptr<glim::CloudPreprocessor> m_preprocessor;
		std::shared_ptr<glim::OdometryEstimationBase> m_odometry;
		std::shared_ptr<glim::SubMappingBase> m_subMapping;
		std::shared_ptr<glim::GlobalMappingBase> m_globalMapping;
		std::shared_ptr<glim::RawPoints> m_pendingFrame;
		_PointCloud *m_pGlobalMap = nullptr;
		vector<std::shared_ptr<glim::SubMap>> m_submaps;
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
		double m_submapStamp = -1.0;
	};
}
#endif
