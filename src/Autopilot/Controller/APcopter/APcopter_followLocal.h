#ifndef OpenKAI_src_Autopilot_Controller_APcopter_followLocal_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_followLocal_H_

#include "APcopter_followBase.h"
#include "APcopter_posCtrlBase.h"

namespace kai
{

class APcopter_followLocal: public APcopter_followBase
{
public:
	APcopter_followLocal();
	~APcopter_followLocal();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

public:
	APcopter_posCtrlBase* m_pPC;
	vDouble4 m_vCam;
};

}
#endif
