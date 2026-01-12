
#ifndef OpenKAI_src_Autopilot__APmavlink_depthVision_H_
#define OpenKAI_src_Autopilot__APmavlink_depthVision_H_

#include "../../Vision/RGBD/_RGBDbase.h"
#include "_APmavlink_base.h"

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

	class _APmavlink_depthVision : public _ModuleBase
	{
	public:
		_APmavlink_depthVision();
		~_APmavlink_depthVision();

		bool init(const json& j);
		void update(void);
		void draw(void *pFrame);

	private:
		_APmavlink_base *m_pAP;
		_RGBDbase *m_pDV;

		int m_nROI;
		DEPTH_ROI m_pROI[N_DEPTH_ROI];
	};

}
#endif
