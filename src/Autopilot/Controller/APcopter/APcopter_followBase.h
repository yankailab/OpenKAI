#ifndef OpenKAI_src_Autopilot_Controller_APcopter_followBase_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_followBase_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../../../Navigation/GPS.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_posCtrlRC.h"

namespace kai
{

class APcopter_followBase: public ActionBase
{
public:
	APcopter_followBase();
	~APcopter_followBase();

	virtual	bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool cli(int& iY);
	virtual int check(void);

	virtual bool find(void);
	virtual void updateGimbal(void);

public:
	APcopter_base* 	m_pAP;
	_ObjectBase*	m_pDet;
	uint64_t		m_tStampDet;
	int				m_iClass;
	_TrackerBase*	m_pTracker;
	bool			m_bUseTracker;

	vDouble3 m_vGimbalReal;
	vDouble3 m_vGimbalOut;		//x:pitch, y:roll, z:yaw
	vDouble3 m_vGimbalRad;
	vDouble4 m_vTarget;
};

}
#endif
