/*
 * _GLIM.h
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__GLIM_H_
#define OpenKAI_src_SLAM__GLIM_H_

#include "_SLAMbase.h"

namespace glim
{
	class CloudPreprocessor;
	class OdometryEstimationBase;
	class SubMappingBase;
	class GlobalMappingBase;
	struct RawPoints;
	struct EstimationFrame;
}

namespace kai
{
	class _GLIM : public _SLAMbase
	{
	public:
		_GLIM();
		~_GLIM() override;
		bool init(const json &j) override;
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
	};
}
#endif
