#ifndef OpenKAI_src_Autopilot_AP__AP_actuator_H_
#define OpenKAI_src_Autopilot_AP__AP_actuator_H_

#include "../../_AutopilotBase.h"
#include "../../../Actuator/_ActuatorBase.h"
#include "../../../Protocol/_Mavlink.h"

namespace kai
{

struct AP_actuator
{
	_ActuatorBase* m_pA;
	int16_t* m_pRCinScaled;
	float	m_k;
	vFloat4 m_vPos;
	vFloat4 m_vSpeed;
	int		m_iMode;

	void init(void)
	{
		m_pA = NULL;
		m_pRCinScaled = NULL;
		m_k = 1.0;
		m_vPos.init(-1.0);
		m_vSpeed.init(1.0);
		m_iMode = 0;
	}

	void update(void)
	{
		NULL_(m_pA);
		NULL_(m_pRCinScaled);
		IF_(*m_pRCinScaled > MAV_RC_SCALE);
		IF_(*m_pRCinScaled < -MAV_RC_SCALE);

		m_vPos.x = ((float)(*m_pRCinScaled))*OV_MAV_RC_SCALE;
		m_pA->moveTo(m_vPos, m_vSpeed);
	}
};

class _AP_actuator: public _AutopilotBase
{
public:
	_AP_actuator();
	~_AP_actuator();

	bool init(void* pKiss);
	bool start(void);
	int	 check(void);
	void update(void);
	void draw(void);

private:
	void updateActuator(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_actuator*) This)->update();
		return NULL;
	}

private:
	_Mavlink* m_pMav;
	vector<AP_actuator> m_vActuator;
	int16_t* m_pRCmode;
	int m_iMode;

};

}

#endif

