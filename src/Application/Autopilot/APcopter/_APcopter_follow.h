#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_follow_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_follow_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../../_ActionBase.h"
#include "_APcopter_base.h"
#include "_APcopter_posCtrl.h"

namespace kai
{

class _APcopter_follow: public _ActionBase
{
public:
	_APcopter_follow();
	~_APcopter_follow();

	virtual	bool init(void* pKiss);
	virtual int check(void);
	virtual bool start(void);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual bool find(void);

private:
	void updateTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_follow *) This)->update();
		return NULL;
	}

public:
	_APcopter_base*	m_pAP;
	_APcopter_posCtrl* m_pPC;
	_DetectorBase*	m_pDet;

	OBJECT			m_tO;
	int				m_iClass;
	int				m_apMode;

	vFloat4 		m_vP;		//variable screen coordinate of the object being followed in roll, pitch, alt, yaw order
	vFloat4 		m_vTargetP;	//constant screen coordinate where the followed object should get to

	bool			m_bUseTracker;
	_TrackerBase*	m_pTracker[2];
	_TrackerBase*	m_pTnow;
	_TrackerBase*	m_pTnew;
	int				m_iTracker;

	AP_MOUNT		m_apMount;
};

}
#endif
