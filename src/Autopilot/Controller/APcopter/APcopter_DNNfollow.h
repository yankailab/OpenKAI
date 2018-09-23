#ifndef OpenKAI_src_Autopilot_Controller_APcopter_DNNfollow_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_DNNfollow_H_

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

class APcopter_DNNfollow: public ActionBase
{
public:
	APcopter_DNNfollow();
	~APcopter_DNNfollow();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

	OBJECT* newFound(void);
	void updateGimbal(void);

public:
	APcopter_base* m_pAP;
	APcopter_posCtrlRC* m_pPC;
	_ObjectBase* m_pDet;
	uint64_t m_tStampDet;
	_TrackerBase* m_pTracker;
	GPS m_GPS;

	bool	m_bUseTracker;
	int		m_iClass;

	UTM_POS m_utmV;	//global vehicle pos
	vDouble3 m_vCamNEA;
	vDouble3 m_vCamRelNEA;	//relative to camera center to lock the target in local NEA
	vDouble3 m_vTargetNEA;	//following target in local NEA
	vDouble3 m_vMyDestNEA;	//vechiel should be going in local NEA
	vDouble3 m_vGimbal;		//x:pitch, y:roll, z:yaw

	vDouble4 m_vTarget;
};

}
#endif
