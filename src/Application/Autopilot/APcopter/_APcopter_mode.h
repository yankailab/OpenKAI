#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_mode_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_mode_H_

#include "../../../Base/common.h"
#include "../../_ActionBase.h"
#include "_APcopter_base.h"

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

class _APcopter_mode: public _ActionBase
{
public:
	_APcopter_mode();
	~_APcopter_mode();

	bool init(void* pKiss);
	int check(void);
	void update(void);
	bool draw(void);
	bool console(int& iY);

public:
	_APcopter_base* m_pAP;
	vector<AP_MODE_MISSION> m_vMM;
};

}
#endif
