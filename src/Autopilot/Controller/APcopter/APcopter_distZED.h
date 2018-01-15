
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_distZED_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_distZED_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_ZEDdistance.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

#define N_DIST_ZED 32
#define N_DIST_ZED_ROI 16

namespace kai
{

struct DIST_ROI
{
	uint8_t			m_orient;
	vInt4			m_roi;

	void init(void)
	{
		m_orient = 0;
		m_roi.init();
	}
};

struct DIST_ZED
{
	_DistSensorBase* m_pZED;
	int m_nROI;
	DIST_ROI m_pROI[N_DIST_ZED_ROI];

	void init(void)
	{
		m_pZED = NULL;
		m_nROI = 0;
	}
};

class APcopter_distZED: public ActionBase
{
public:
	APcopter_distZED();
	~APcopter_distZED();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	void updateMavlink(void);

	APcopter_base* m_pAP;
	int	m_nZED;
	DIST_ZED m_pZED[N_DIST_ZED];

};

}

#endif

