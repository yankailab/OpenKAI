#ifndef OpenKAI_src_Autopilot_AP__AP_actuator_H_
#define OpenKAI_src_Autopilot_AP__AP_actuator_H_

#include "../../../Actuator/_ActuatorBase.h"
#include "_AP_base.h"

namespace kai
{

struct AP_actuator
{
	_ActuatorBase* m_pA;
	uint8_t m_pRCin[4];
	float	m_pInc[4];
	vFloat4	m_vPWMrange;
	vFloat4 m_vK;
	vFloat4 m_vSpeed;
	int		m_iMode;

	void init(void)
	{
		m_pA = NULL;
		m_vPWMrange.init(1000, 1250, 1750, 2000);
		m_vK.init(1.0);
		m_vSpeed.init(1.0);
		m_iMode = -1;

		for(int i=0;i<4;i++)
		{
			m_pRCin[i] = 0;
			m_pInc[i] = 0.01;
		}
	}

	void update(_Mavlink* pMav)
	{
		NULL_(m_pA);
		NULL_(pMav);

		float pP[4];
		pP[0] = 0.0;
		pP[1] = 0.0;
		pP[2] = 0.0;
		pP[3] = 0.0;

		float pK[4];
		pK[0] = m_vK.x;
		pK[1] = m_vK.y;
		pK[2] = m_vK.z;
		pK[3] = m_vK.w;

		for(int i=0; i<4; i++)
		{
			uint16_t v = pMav->m_rcChannels.getRC(m_pRCin[i]);
			IF_CONT(v < m_vPWMrange.x && v > m_vPWMrange.w);

			pP[i] = (v - m_vPWMrange.x)/(m_vPWMrange.w-m_vPWMrange.x);
			if(pK[i]<0.0)
				pP[i] = 1.0 - pP[i];
		}

		vFloat4 vP;
		vP = pP;
		m_pA->pos(vP);
		m_pA->speed(m_vSpeed);
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
	_AP_base* m_pAP;
	vector<AP_actuator> m_vActuator;
	uint8_t m_iRCmode;
	int m_iMode;
	vFloat4	m_vPWMrange;

};

}

#endif

