/*
 * _ActuatorBase.h
 *
 *  Created on: May 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__ActuatorBase_H_
#define OpenKAI_src_Actuator__ActuatorBase_H_

#include "../Base/_ThreadBase.h"

namespace kai
{

struct ACTUATOR_AXIS
{
	string m_name;

	//normalized
	float m_nP;
	float m_nPtarget;
	float m_nPorigin;
	float m_nPerr;
	float m_nS;
	float m_nStarget;

	//raw
	vFloat2 m_vPrange;
	float m_rawP;
	float m_pOrigin;
	vFloat2 m_vSrange;
	float m_rawS;

	void init(void)
	{
		m_name = "";

		m_nP = -1.0;
		m_nPtarget = -1.0;
		m_nPorigin = -1.0;
		m_nPerr = 0.01;
		m_nS = 0.0;
		m_nStarget = 0.0;

		m_vPrange.init(0.0);
		m_rawP = 0.0;
		m_pOrigin = 0.0;
		m_vSrange.init(0.0);
		m_rawS = 0.0;
	}

	bool bComplete(void)
	{
		IF_T(m_nP < 0.0);
		IF_T(EAQ(m_nP, m_nPtarget, m_nPerr));

		return false;
	}

	void setPtarget(float nP)
	{
		m_nP = constrain(nP, -1.0f, 1.0f);
	}

	void setStarget(float nS)
	{
		nS = constrain(nS, -1.0f, 1.0f);
		if(m_nP >= 0.0 && m_nP <= 1.0)
			m_nStarget = nS;
	}

	void setP(float nP)
	{
		m_nP = nP;
		m_rawP = m_nP * m_vPrange.d() + m_vPrange.x;
	}

	void setS(float nS)
	{
		m_nS = nS;
		m_rawS = m_nS * m_vSrange.d() + m_vSrange.x;
	}

	void gotoOrigin(void)
	{
		setPtarget(m_nPorigin);
	}

	void setRawP(float p)
	{
		m_rawP = p;
		m_nP = (float) (m_rawP - m_vPrange.x) / (float) m_vPrange.len();
	}

	void setRawS(float s)
	{
		m_rawS = s;
		m_nS = (float) (m_rawS - m_vSrange.x) / (float) m_vSrange.len();
	}

	float getP(void)
	{
		return m_nP;
	}

	float getS(void)
	{
		return m_nS;
	}

	float getPtarget(void)
	{
		return m_nPtarget;
	}

	float getStarget(void)
	{
		return m_nStarget;
	}

	float getRawP(void)
	{
		return m_rawP;
	}

	float getRawS(void)
	{
		return m_rawS;
	}

	float getRawPtarget(void)
	{
		return m_nPtarget * m_vPrange.d() + m_vPrange.x;
	}

	float getRawStarget(void)
	{
		return m_nStarget * m_vSrange.d() + m_vSrange.x;
	}

	bool bNormal(void)
	{
		IF_F(m_nP < 0.0);
		IF_F(m_nP > 1.0);

		return true;
	}
};

class _ActuatorBase: public _ThreadBase
{
public:
	_ActuatorBase();
	~_ActuatorBase();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual void draw(void);

	virtual void setPtarget(int i, float nP);
	virtual void setStarget(int i, float nS);
	virtual void gotoOrigin(void);
	virtual bool bComplete(void);

	virtual float getP(int i);
	virtual float getS(int i);
	virtual float getRawP(int i);
	virtual float getRawS(int i);

private:
	virtual bool open(void);
	virtual void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ActuatorBase*) This)->update();
		return NULL;
	}

public:
	vector<ACTUATOR_AXIS> m_vAxis;
	int m_nAxis;

	bool	m_bFeedback;
	_ActuatorBase* m_pParent;

};

}
#endif

