
#ifndef OpenKAI_src_Autopilot__APcopter_depthVision_H_
#define OpenKAI_src_Autopilot__APcopter_depthVision_H_

#include "../../../Base/common.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_base.h"

#define N_DEPTH_ROI 16

namespace kai
{

struct DEPTH_ROI
{
	uint8_t		m_orientation;
	vFloat4		m_roi;
	float		m_minD;

	void init(void)
	{
		m_minD = 0.0;
		m_orientation = 0;
		m_roi.init();
	}
};

class _APcopter_depthVision: public _AutopilotBase
{
public:
	_APcopter_depthVision();
	~_APcopter_depthVision();

	bool init(void* pKiss);
	void update(void);
	void draw(void);

private:
	_APcopter_base* m_pAP;
	_DepthVisionBase* m_pDV;

	int m_nROI;
	DEPTH_ROI m_pROI[N_DEPTH_ROI];

};

}

#endif
