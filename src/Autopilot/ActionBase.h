
#ifndef OpenKAI_src_Autopilot_ActionBase_H_
#define OpenKAI_src_Autopilot_ActionBase_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Mission/_MissionControl.h"
#include "../UI/Window.h"

#define N_ACTIVESTATE 128

namespace kai
{

class ActionBase: public BASE
{
public:
	ActionBase();
	~ActionBase();

	virtual bool init(void* pKiss);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool isActive(void);
	virtual bool isStateChanged(void);
	virtual int check(void);

public:
	_MissionControl* m_pAM;
	std::vector<int>	m_vActiveState;
	int m_iLastState;
	bool m_bStateChanged;

	int64_t m_tStamp;
	int64_t m_dTime;
	int		m_iPriority;

};

}
#endif
