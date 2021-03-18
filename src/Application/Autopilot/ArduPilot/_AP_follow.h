#ifndef OpenKAI_src_Autopilot_AP__AP_follow_H_
#define OpenKAI_src_Autopilot_AP__AP_follow_H_

#include "../../../Universe/_Universe.h"
#include "../../../Tracker/_TrackerBase.h"
#include "../ArduPilot/_AP_posCtrl.h"
#include "../../../Filter/Median.h"
#include "../../../Filter/Average.h"
#include "../../../Filter/Predict.h"
#include "../../../Filter/Hold.h"

#ifdef USE_OPENCV

namespace kai
{

struct FOLLOW_TARGET_FILT
{
	Median<float> m_med;
	Predict<float> m_pred;
	Hold<float> m_hold;

	bool init(int nWmed, int nWpred, float dThold, float vTover)
	{
		IF_F(!m_med.init(nWmed));
		IF_F(!m_pred.init(nWpred));
		IF_F(!m_hold.init(dThold, vTover));

		return true;
	}

	void reset(void)
	{
		m_med.reset();
		m_pred.reset();
		m_hold.reset();
	}

	float input(float v, bool b, float dTsec)
	{
		return m_pred.input(m_med.input(m_hold.input(v,b,dTsec) ), dTsec);
	}
};

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
	static void* getUpdate(void* This)
	{
		((_AP_follow *) This)->update();
		return NULL;
	}

public:
	_Universe*     	m_pU;
	_TrackerBase*	m_pTracker;
	int				m_iClass;
	bool			m_bTarget;
	vFloat4			m_vTargetBB;

	FOLLOW_TARGET_FILT m_fX;
	FOLLOW_TARGET_FILT m_fY;
	FOLLOW_TARGET_FILT m_fW;

	AP_MOUNT		m_apMount;

//	INTERVAL_EVENT m_ieSend;
};

}
#endif
#endif
