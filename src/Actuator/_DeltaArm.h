#ifndef OpenKAI_src_Actuator__DeltaArm_H_
#define OpenKAI_src_Actuator__DeltaArm_H_

#include "_ActuatorBase.h"
#include "../Dependency/Rasheed/RasheedDeltaRobot.h"

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
	virtual bool draw(void);
	virtual bool console(int& iY);

private:
	void updateMotion(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DeltaArm *) This)->update();
		return NULL;
	}

public:
	RasheedDeltaRobot m_rdr;

};

}
#endif

#endif
