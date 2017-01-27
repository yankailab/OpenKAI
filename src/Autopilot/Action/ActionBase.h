
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_ACTIONBASE_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_ACTIONBASE_H_

#include "../../Base/common.h"
#include "../../Base/BASE.h"
#include "../../Automaton/_Automaton.h"
#include "../../UI/Window.h"

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

public:
	_Automaton* m_pAM;
	std::vector<int>	m_vActiveState;

	int64_t m_timeStamp;
	int64_t m_dTime;

};

}

#endif

