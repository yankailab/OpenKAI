#ifndef OpenKAI_src_Autopilot_Controller_APcopter_tracker_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_tracker_H_

#include "../../../../Base/common.h"
#include "../../../../Detector/_DetectorBase.h"
#include "../../../../Tracker/_TrackerBase.h"
#include "../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_posCtrl.h"

namespace kai
{

class APcopter_tracker: public ActionBase
{
public:
	APcopter_tracker();
	~APcopter_tracker();

	virtual	bool init(void* pKiss);
	virtual int check(void);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool console(int& iY);

public:
	APcopter_base*	m_pAP;
	APcopter_posCtrl* m_pPC;
	_TrackerBase*	m_pT;

	int				m_apMode;
	vFloat4			m_tBB;
	vDouble4 		m_vTargetPos; 	//roll, pitch, alt, yaw
	vDouble4 		m_vMyPos;

	AP_MOUNT		m_apMount;
};

}
#endif
