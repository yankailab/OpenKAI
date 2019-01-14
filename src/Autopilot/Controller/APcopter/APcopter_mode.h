#ifndef OpenKAI_src_Autopilot_Controller_APcopter_mode_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_mode_H_

#include "../../../Base/common.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_mode: public ActionBase
{
public:
	APcopter_mode();
	~APcopter_mode();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	bool draw(void);
	bool console(int& iY);

public:
	APcopter_base* m_pAP;

	uint64_t m_tStart;
	uint64_t m_tReturn;
	uint16_t m_iMissionSeq;

};

}
#endif
