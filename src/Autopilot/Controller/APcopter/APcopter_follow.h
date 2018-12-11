#ifndef OpenKAI_src_Autopilot_Controller_APcopter_follow_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_follow_H_

#include "../../../Base/common.h"
#include "../../../Base/_ObjectBase.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../../../Navigation/GPS.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"
#include "APcopter_posCtrlBase.h"

namespace kai
{

class APcopter_follow: public ActionBase
{
public:
	APcopter_follow();
	~APcopter_follow();

	virtual	bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual int check(void);
	virtual void update(void);

	virtual bool find(void);
	virtual void updateGimbal(void);

public:
	APcopter_base* 	m_pAP;
	APcopter_posCtrlBase* m_pPC;
	_ObjectBase*	m_pDet;
	uint64_t		m_tStampDet;
	int				m_iClass;

	bool			m_bUseTracker;
	_TrackerBase*	m_pTracker[2];
	_TrackerBase*	m_pTnow;
	_TrackerBase*	m_pTnew;
	int				m_iTracker;

	vDouble3 m_vGimbal;		//x:pitch, y:roll, z:yaw
	vDouble4 m_vTarget;

	mavlink_mount_control_t m_mountControl;
	mavlink_mount_configure_t m_mountConfig;

};

}
#endif
