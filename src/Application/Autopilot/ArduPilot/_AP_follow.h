#ifndef OpenKAI_src_Autopilot_AP__AP_follow_H_
#define OpenKAI_src_Autopilot_AP__AP_follow_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../ArduPilot/_AP_base.h"
#include "../ArduPilot/_AP_posCtrl.h"

namespace kai
{

class _AP_follow: public _AP_posCtrl
{
public:
	_AP_follow();
	~_AP_follow();

	virtual	bool init(void* pKiss);
	virtual int check(void);
	virtual bool start(void);
	virtual void update(void);
	virtual void draw(void);

protected:
	bool updateTarget(void);
	virtual bool findTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_follow *) This)->update();
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
