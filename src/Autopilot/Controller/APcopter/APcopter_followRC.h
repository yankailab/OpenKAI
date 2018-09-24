#ifndef OpenKAI_src_Autopilot_Controller_APcopter_followRC_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_followRC_H_

#include "APcopter_followBase.h"
#include "APcopter_posCtrlRC.h"

namespace kai
{

class APcopter_followRC: public APcopter_followBase
{
public:
	APcopter_followRC();
	~APcopter_followRC();

	bool init(void* pKiss);
	void update(void);
	bool draw(void);
	bool cli(int& iY);
	int check(void);

public:
	APcopter_posCtrlRC* m_pPC;
};

}
#endif
