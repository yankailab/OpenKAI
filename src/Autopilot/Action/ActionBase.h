
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_ACTIONBASE_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_ACTIONBASE_H_

#include "../../Base/common.h"

namespace kai
{

class ActionBase: public BASE
{
public:
	ActionBase();
	~ActionBase();

	bool init(Config* pConfig);
	bool link(void);
	void update(void);

public:

};

}

#endif

