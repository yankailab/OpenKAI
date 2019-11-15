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
	RasheedDeltaRobot m_rdr;
	int m_oprMode;
	bool m_bGripper;

	vFloat3 m_vAngle;
	vFloat3 m_vPos;
	vFloat2	m_vPosRangeX;
	vFloat2	m_vPosRangeY;
	vFloat2	m_vPosRangeZ;

};

}
#endif

#endif
