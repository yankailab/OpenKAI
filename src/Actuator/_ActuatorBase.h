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

	//pos
	float m_p;
	float m_pTarget;
	float m_pOrigin;
	float m_pEerr;
	vFloat2 m_vRawPrange;
	float m_rawP;
	float m_rawPorigin;

	//speed
	float m_s;
	float m_sTarget;
	vFloat2 m_vRawSrange;
	float m_rawS;

	//accel
	float m_aTarget;
	vFloat2 m_vRawArange;
	float m_rawA;

	void init(void)
	{
		m_name = "";

		m_p = -1.0;
		m_pTarget = -1.0;
		m_pOrigin = -1.0;
		m_pEerr = 0.01;
		m_vRawPrange.init(0.0);
		m_rawP = 0.0;
		m_rawPorigin = 0.0;

		m_s = 0.0;
		m_sTarget = 0.0;
		m_vRawSrange.init(0.0);
		m_rawS = 0.0;

		m_aTarget = 0.0;
		m_vRawArange.init(0.0);
		m_rawA = 0.0;
	}

	bool bComplete(void)
	{
		IF_T(m_p < 0.0);
		IF_T(EAQ(m_p, m_pTarget, m_pEerr));

		return false;
	}

	void setPtarget(float nPt)
	{
		nPt = constrain(nPt, -1.0f, 1.0f);
		m_pTarget = nPt;
	}

	void setStarget(float nSt)
	{
		nSt = constrain(nSt, -1.0f, 1.0f);
		m_sTarget = nSt;
	}

	void setAtarget(float nAt)
	{
		nAt = constrain(nAt, -1.0f, 1.0f);
		m_aTarget = nAt;
	}

	void setP(float nP)
	{
		m_p = nP;
		m_rawP = m_p * m_vRawPrange.d() + m_vRawPrange.x;
	}

	void setS(float nS)
	{
		m_s = nS;
		m_rawS = m_s * m_vRawSrange.d() + m_vRawSrange.x;
	}

	void gotoOrigin(void)
	{
		setPtarget(m_pOrigin);
	}

	void setRawP(float p)
	{
		m_rawP = p;
		m_p = (float) (m_rawP - m_vRawPrange.x) / (float) m_vRawPrange.len();
	}

	void setRawS(float s)
	{
		m_rawS = s;
		m_s = (float) (m_rawS - m_vRawSrange.x) / (float) m_vRawSrange.len();
	}

	float getP(void)
	{
		return m_p;
	}

	float getS(void)
	{
		return m_s;
	}

	float getPtarget(void)
	{
		return m_pTarget;
	}

	float getStarget(void)
	{
		return m_sTarget;
	}

	float getAtarget(void)
	{
		return m_aTarget;
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
		return m_pTarget * m_vRawPrange.d() + m_vRawPrange.x;
	}

	float getRawStarget(void)
	{
		return m_sTarget * m_vRawSrange.d() + m_vRawSrange.x;
	}

	bool bNormal(void)
	{
		IF_F(m_p < 0.0);
		IF_F(m_p > 1.0);

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
	int m_nMinAxis;

	bool	m_bFeedback;
	_ActuatorBase* m_pParent;

};

}
#endif

