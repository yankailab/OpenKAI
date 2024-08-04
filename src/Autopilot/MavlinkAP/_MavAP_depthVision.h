
#ifndef OpenKAI_src_Autopilot__MavAP_depthVision_H_
#define OpenKAI_src_Autopilot__MavAP_depthVision_H_

#include "../../Vision/RGBD/_RGBDbase.h"
#include "_MavAP_base.h"

#define N_DEPTH_ROI 16

namespace kai
{

	struct DEPTH_ROI
	{
		uint8_t m_orientation;
		vFloat4 m_roi;
		float m_minD;

		void init(void)
		{
			m_minD = 0.0;
			m_orientation = 0;
			m_roi.clear();
		}
	};

	class _MavAP_depthVision : public _ModuleBase
	{
	public:
		_MavAP_depthVision();
		~_MavAP_depthVision();

		int init(void *pKiss);
		void update(void);
		void draw(void *pFrame);

	private:
		_MavAP_base *m_pAP;
		_RGBDbase *m_pDV;

		int m_nROI;
		DEPTH_ROI m_pROI[N_DEPTH_ROI];
	};

}
#endif
