#ifndef OpenKAI_src_Autopilot_Controller_APcopter_avoid_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_avoid_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_posCtrl.h"

namespace kai
{

class APcopter_avoid: public ActionBase
{
public:
	APcopter_avoid();
	~APcopter_avoid();

	virtual	bool init(void* pKiss);
	virtual int check(void);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual bool find(void);
	virtual bool bFound(void);

public:
	APcopter_base*	m_pAP;
	APcopter_posCtrl* m_pPC;
	_DetectorBase*	m_pDet;

	OBJECT			m_tO;
	int				m_iClass;
	bool			m_bFound;
	int				m_apMode;

	vDouble4 		m_vTargetPos; 	//roll, pitch, alt, yaw
	vDouble4 		m_vMyPos;

	bool			m_bUseTracker;
	_TrackerBase*	m_pTracker[2];
	_TrackerBase*	m_pTnow;
	_TrackerBase*	m_pTnew;
	int				m_iTracker;

	AP_MOUNT		m_apMount;
};

}
#endif
