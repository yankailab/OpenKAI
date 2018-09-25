#ifndef OpenKAI_src_Autopilot_Controller_APcopter_followGlobal_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_followGlobal_H_

#include "../../../Navigation/GPS.h"
#include "APcopter_followBase.h"

namespace kai
{

class APcopter_followGlobal: public APcopter_followBase
{
public:
	APcopter_followGlobal();
	~APcopter_followGlobal();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

public:
	GPS m_GPS;

	UTM_POS	 m_utmV;	//global vehicle pos
	vDouble3 m_vCamNEA;
	vDouble3 m_vCamRelNEA;	//relative to camera center to lock the target in local NEA
	vDouble3 m_vTargetNEA;	//following target in local NEA
	vDouble3 m_vMyDestNEA;	//vechiel should be going in local NEA
	vDouble3 m_vMove;
};

}
#endif
