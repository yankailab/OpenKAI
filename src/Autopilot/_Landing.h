
#ifndef OPENKAI_SRC_AUTOPILOT__LANDING_H_
#define OPENKAI_SRC_AUTOPILOT__LANDING_H_

#include "../Base/common.h"
#include "_AutoPilot.h"

namespace kai
{

class _Landing: public _AutoPilot
{
public:
	_Landing();
	~_Landing();

	bool init(Config* pConfig, string name);
	bool start(void);
	bool draw(Frame* pFrame, iVector4* pTextPos);

public:

	void landingTarget(void);


	//Thread
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Landing *) This)->update();
		return NULL;
	}

};

}

#endif

