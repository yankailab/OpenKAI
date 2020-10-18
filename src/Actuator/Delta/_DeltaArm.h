#ifndef OpenKAI_src_Actuator__DeltaArm_H_
#define OpenKAI_src_Actuator__DeltaArm_H_

#include "../../Dependency/Rasheed/DeltaRobot.h"
#include "../_ActuatorBase.h"

#ifdef USE_DYNAMIXEL

namespace kai
{

class _DeltaArm: public _ActuatorBase
{
public:
	_DeltaArm();
	~_DeltaArm();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual void draw(void);

private:
	void updatePos(void);
	void readStatus(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DeltaArm *) This)->update();
		return NULL;
	}

public:
	DeltaRobot m_dr;
	int m_oprMode;
	bool m_bGripper;
};

}
#endif

#endif
