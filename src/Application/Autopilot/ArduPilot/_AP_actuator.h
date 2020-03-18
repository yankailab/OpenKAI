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
	uint16_t* m_ppRCin[4];
	float	m_pInc[4];
	vFloat4	m_vPWMrange;
	vFloat4 m_vSpeed;
	int		m_iMode;

	void init(void)
	{
		m_pA = NULL;
		m_vPWMrange.init(1100, 1250, 1750, 1900);
		m_vSpeed.init(1.0);
		m_iMode = -1;

		for(int i=0;i<4;i++)
		{
			m_ppRCin[i] = NULL;
			m_pInc[i] = 0.01;
		}
	}

	void update(void)
	{
		NULL_(m_pA);

		vFloat4 vP = m_pA->getPos();
		float pP[4];
		pP[0] = vP.x;
		pP[1] = vP.y;
		pP[2] = vP.z;
		pP[3] = vP.w;

		for(int i=0; i<4; i++)
		{
			IF_CONT(!m_ppRCin[i]);

			uint16_t v = *(m_ppRCin[i]);
			IF_CONT(v < m_vPWMrange.x && v > m_vPWMrange.w);

			if(v > m_vPWMrange.z)
				pP[i] += m_pInc[i];
			else if(v < m_vPWMrange.y)
				pP[i] -= m_pInc[i];
		}

		vP = pP;
		m_pA->moveTo(vP, m_vSpeed);
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
	uint16_t* m_pRCmode;
	int m_iMode;
	vFloat4	m_vPWMrange;

};

}

#endif

