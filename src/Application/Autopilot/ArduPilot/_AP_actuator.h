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
	int16_t* m_ppRCin[4];
	float	m_k;
	vFloat4 m_vPos;
	vFloat4 m_vSpeed;
	int		m_iMode;

	void init(void)
	{
		m_pA = NULL;
		m_ppRCin[0] = NULL;
		m_ppRCin[1] = NULL;
		m_ppRCin[2] = NULL;
		m_ppRCin[3] = NULL;
		m_k = 1.0;
		m_vPos.init(-1.0);
		m_vSpeed.init(1.0);
		m_iMode = -1;
	}

	void update(void)
	{
		NULL_(m_pA);

		int16_t v;

		if(m_ppRCin[0])
		{
			v = *(m_ppRCin[0]);
			if(v >= -MAV_RC_SCALE && v <= MAV_RC_SCALE)
				m_vPos.x = ((float)(MAV_RC_SCALE+v))*OV_MAV_RC_SCALE*0.5;
			else
				m_vPos.x = -1.0;
		}

		if(m_ppRCin[1])
		{
			v = *(m_ppRCin[1]);
			if(v >= -MAV_RC_SCALE && v <= MAV_RC_SCALE)
				m_vPos.y = ((float)(MAV_RC_SCALE+v))*OV_MAV_RC_SCALE*0.5;
			else
				m_vPos.y = -1.0;
		}

		if(m_ppRCin[2])
		{
			v = *(m_ppRCin[2]);
			if(v >= -MAV_RC_SCALE && v <= MAV_RC_SCALE)
				m_vPos.z = ((float)(MAV_RC_SCALE+v))*OV_MAV_RC_SCALE*0.5;
			else
				m_vPos.z = -1.0;
		}

		if(m_ppRCin[3])
		{
			v = *(m_ppRCin[3]);
			if(v >= -MAV_RC_SCALE && v <= MAV_RC_SCALE)
				m_vPos.w = ((float)(MAV_RC_SCALE+v))*OV_MAV_RC_SCALE*0.5;
			else
				m_vPos.w = -1.0;
		}

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

