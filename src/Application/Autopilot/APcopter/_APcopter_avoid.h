
#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_avoid_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_avoid_H_

#include "../../../Detector/_DetectorBase.h"
#include "_APcopter_base.h"

namespace kai
{

class _APcopter_avoid: public _AutopilotBase
{
public:
	_APcopter_avoid();
	~_APcopter_avoid();

	bool init(void* pKiss);
	bool start(void);
	int	 check(void);
	void update(void);
	void draw(void);

private:
	void updateTarget(void);
	static void* getUpdateThread(void* This)
	{
		((_APcopter_avoid*) This)->update();
		return NULL;
	}

private:
	_APcopter_base* m_pAP;
	_DetectorBase* m_pDet;
	_Mavlink* m_pMavlink;

	OBJECT m_obs;
};

}

#endif

