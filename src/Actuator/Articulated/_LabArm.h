#ifndef OpenKAI_src_Actuator_Articulated__LabArm_H_
#define OpenKAI_src_Actuator_Articulated__LabArm_H_

#include "../_ActuatorBase.h"
#include "../../Dependency/Rasheed/LabArm.h"

#ifdef USE_DYNAMIXEL

namespace kai
{

class _LabArm: public _ActuatorBase
{
public:
	_LabArm();
	~_LabArm();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual void draw(void);

private:
	void updatePos(void);
	void readStatus(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_LabArm *) This)->update();
		return NULL;
	}

public:
	LabArm m_la;
	int m_oprMode;
	bool m_bGripper;
};

}
#endif
#endif
