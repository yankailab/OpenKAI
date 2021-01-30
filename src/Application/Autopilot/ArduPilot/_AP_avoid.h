
#ifndef OpenKAI_src_Autopilot_AP__AP_avoid_H_
#define OpenKAI_src_Autopilot_AP__AP_avoid_H_

#include "../../../Detector/_DetectorBase.h"
#include "../ArduPilot/_AP_base.h"

#ifdef USE_OPENCV

namespace kai
{

class _AP_avoid: public _StateBase
{
public:
	_AP_avoid();
	~_AP_avoid();

	bool init(void* pKiss);
	bool start(void);
	int	 check(void);
	void update(void);
	void draw(void);

private:
	void updateTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_avoid*) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	_DetectorBase* m_pDet;
	_Mavlink* m_pMavlink;

	_Object m_obs;
};

}
#endif
#endif

