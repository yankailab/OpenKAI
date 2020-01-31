#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_follow_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_follow_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_base.h"
#include "_APcopter_link.h"
#include "_APcopter_posCtrl.h"

namespace kai
{

class _APcopter_follow: public _APcopter_posCtrl
{
public:
	_APcopter_follow();
	~_APcopter_follow();

	virtual	bool init(void* pKiss);
	virtual int check(void);
	virtual bool start(void);
	virtual void update(void);
	virtual void draw(void);

	bool updateFollow(void);
	virtual bool updateTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_follow *) This)->update();
		return NULL;
	}

public:
	_DetectorBase*	m_pDet;
	_TrackerBase*	m_pT;
	int				m_iClass;
	bool			m_bTarget;
	vFloat4			m_vTargetBB;

	AP_MOUNT		m_apMount;

//	INTERVAL_EVENT m_ieSend;
};

}
#endif
