#ifndef OpenKAI_src_Autopilot_Controller_APcopter_mode_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_mode_H_

#include "../../../../Base/common.h"
#include "../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

struct AP_MODE_MISSION
{
	int m_fromApMode;
	int m_fromApMissionSeq;

	int m_toApMode;
	int m_toMissionIdx;

	void init(void)
	{
		m_fromApMode = -1;
		m_fromApMissionSeq = -1;

		m_toApMode = -1;
		m_toMissionIdx = -1;
	}
};

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
	vector<AP_MODE_MISSION> m_vMM;
};

}
#endif
