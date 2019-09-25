#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_follow_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_follow_H_

#include "../../../Base/common.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../../_ActionBase.h"
#include "_APcopter_base.h"
#include "_APcopter_link.h"
#include "_APcopter_posCtrl.h"

namespace kai
{

#define APFOLLOW_OFF 0
#define APFOLLOW_ON 1

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

private:
	void updateTargetPos(void);
	void updatePos(void);
	void sendClient(void);
	bool updateBBclient(vFloat4* pBB);
	bool updateBBdet(vFloat4* pBB);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_follow *) This)->update();
		return NULL;
	}

public:
	_APcopter_base*	m_pAP;
	_APcopter_link* m_pAL;
	_APcopter_posCtrl* m_pPC;
	_DetectorBase*	m_pDet;
	AP_MOUNT		m_apMount;

	OBJECT			m_tO;
	int				m_iClass;
	uint64_t		m_timeOut;

	vFloat4			m_vBB;
	vFloat4 		m_vP;		//variable screen coordinate of the object being followed in roll, pitch, alt, yaw order
	vFloat4 		m_vTargetP;	//constant screen coordinate where the followed object should get to
	vFloat2			m_vTargetPregion;
	vFloat2			m_vKtarget;
	vFloat4 		m_vTargetOrigin;	//constant screen coordinate where the followed object should get to

	bool			m_bBB;

	_TrackerBase*	m_pT;
	INTERVAL_EVENT m_ieSend;


};

}
#endif
