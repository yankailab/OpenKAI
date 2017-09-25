
#ifndef OpenKAI_src_Autopilot_ActionBase_H_
#define OpenKAI_src_Autopilot_ActionBase_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Automaton/_Automaton.h"
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
	virtual bool link(void);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool isActive(void);
	virtual bool isStateChanged(void);

public:
	_Automaton* m_pAM;
	std::vector<int>	m_vActiveState;
	int m_iLastState;
	bool m_bStateChanged;

	int64_t m_timeStamp;
	int64_t m_dTime;

};

}

#endif

