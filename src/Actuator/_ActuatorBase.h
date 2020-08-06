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

struct ACTUATOR_AXIS_PARAM
{
	float m_v;
	float m_vTarget;
	float m_vErr;
	vFloat2 m_vRawRange;

	void init(void)
	{
		m_v = -1.0;
		m_vTarget = -1.0;
		m_vErr = 0.0;
		m_vRawRange.init(0.0);
	}

	bool bComplete(void)
	{
		IF_T(m_v < 0.0);
		IF_T(EQUAL(m_v, m_vTarget, m_vErr));

		return false;
	}

	bool bValid(void)
	{
		IF_F(m_v < 0.0);
		IF_F(m_v > 1.0);

		return true;
	}

	void setTarget(float nV)
	{
		m_vTarget = constrain(nV, -1.0f, 1.0f);
	}

	void setTargetRaw(float rV)
	{
		float v = (float) (rV - m_vRawRange.x) / (float) m_vRawRange.len();
		m_vTarget = constrain(v, -1.0f, 1.0f);
	}

	void set(float nV)
	{
		m_v = nV;
	}

	void setRaw(float rV)
	{
		m_v = (float) (rV - m_vRawRange.x) / (float) m_vRawRange.len();
	}

	float getRaw(void)
	{
		return m_v * m_vRawRange.d() + m_vRawRange.x;
	}

	float getTargetRaw(void)
	{
		return m_vTarget * m_vRawRange.d() + m_vRawRange.x;
	}
};

struct ACTUATOR_AXIS
{
	string m_name;

	float m_pOrigin;
	ACTUATOR_AXIS_PARAM m_p;	//pos
	ACTUATOR_AXIS_PARAM m_s;	//speed
	ACTUATOR_AXIS_PARAM m_a;	//accel
	ACTUATOR_AXIS_PARAM m_b;	//brake
	ACTUATOR_AXIS_PARAM m_c;	//current

	void init(void)
	{
		m_name = "";

		m_pOrigin = 0.0;
		m_p.init();
		m_s.init();
		m_a.init();
		m_b.init();
		m_c.init();
	}

	void gotoOrigin(void)
	{
		m_p.setTarget(m_pOrigin);
	}
};

enum ACTUATOR_CMD_TYPE
{
	actCmd_standby,
	actCmd_pos,
	actCmd_spd,
	actCmd_posSpd,
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
	virtual void setPtargetRaw(int i, float rawP);
	virtual void setStarget(int i, float nS);
	virtual void gotoOrigin(void);
	virtual void setCmd(ACTUATOR_CMD_TYPE cType);
	virtual bool bComplete(void);
	virtual bool bComplete(int i);

	virtual float getP(int i);
	virtual float getS(int i);
	virtual float getPraw(int i);
	virtual float getSraw(int i);

protected:
	virtual bool bCmdTimeout(void);
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

	bool m_bFeedback;
	bool m_bMoving;

	ACTUATOR_CMD_TYPE m_lastCmdType;
	uint64_t m_tLastCmd;
	uint64_t m_tCmdTimeout;

	_ActuatorBase* m_pParent;

};

}
#endif

