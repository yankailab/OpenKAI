#ifndef OpenKAI_src_Actuator_Articulated__S6H4D_H_
#define OpenKAI_src_Actuator_Articulated__S6H4D_H_

#include "../_ActuatorBase.h"
#include "../../IO/_IOBase.h"

#define S6H4D_CMD_N 48
#define S6H4D_MOV_BEGIN 0xee
#define S6H4D_MOV_END 0xef
#define S6H4D_CTRL_BEGIN 0xfc
#define S6H4D_CTRL_END 0xfd
#define S6H4D_STAT_BEGIN 0xce
#define S6H4D_STAT_END 0xcf

namespace kai
{

struct S6H4D_CMD_MOV
{
	uint8_t m_b[S6H4D_CMD_N];

	void init(uint8_t a, uint8_t b)
	{
		memset(m_b, 0, S6H4D_CMD_N);
		m_b[0] = S6H4D_MOV_BEGIN;
		m_b[1] = a;
		m_b[2] = b;
		m_b[47] = S6H4D_MOV_END;
	}

	void set(vFloat3 &vP, vFloat3 &vR, float spd)
	{
		f2b(&m_b[3], vP.x);
		f2b(&m_b[7], vP.y);
		f2b(&m_b[11], vP.z);
		f2b(&m_b[15], vR.x);
		f2b(&m_b[19], vR.y);
		f2b(&m_b[23], vR.z);
		f2b(&m_b[27], 1500); //pwm
		f2b(&m_b[31], 0);
		f2b(&m_b[35], 0);
		f2b(&m_b[39], 0);
		f2b(&m_b[43], spd);
	}
};

struct S6H4D_CMD_CTRL
{
	uint8_t m_b[S6H4D_CMD_N];

	void init(void)
	{
		memset(m_b, 0, S6H4D_CMD_N);
		m_b[0] = S6H4D_CTRL_BEGIN;
		m_b[47] = S6H4D_CTRL_END;
	}
};

struct S6H4D_CMD_STATE
{
	uint8_t m_pB[9];
	int m_iB;

	void init(void)
	{
		m_iB = 0;
	}
};

enum S6H4D_VOL_TYPE
{
	vol_box = 0,
	vol_ball = 1,
	vol_cylinder = 2,
};

struct S6H4D_VOL
{
	S6H4D_VOL_TYPE m_type;
	bool	m_bInside;	//true: inside valid

	vFloat2 m_vX;
	vFloat2 m_vY;
	vFloat2 m_vZ;

	vFloat3 m_vC;
	vFloat2 m_vR;

	void init(void)
	{
		m_type = vol_ball;
		m_bInside = false;
		m_vX.init(0.0);
		m_vY.init(0.0);
		m_vZ.init(0.0);
		m_vC.init(0.0);
		m_vR.init(0.0);
	}

	bool bValid(vFloat3& vP)
	{
		bool bInside = true;

		if(m_type == vol_box)
		{
			if(vP.x < m_vX.x)bInside = false;
			if(vP.x > m_vX.y)bInside = false;
			if(vP.y < m_vY.x)bInside = false;
			if(vP.y > m_vY.y)bInside = false;
			if(vP.z < m_vZ.x)bInside = false;
			if(vP.z > m_vZ.y)bInside = false;
		}
		else if(m_type == vol_ball)
		{
			vFloat3 vR = vP - m_vC;
			float r = vR.r();
			if(r < m_vR.x)bInside = false;
			if(r > m_vR.y)bInside = false;
		}
		else if(m_type == vol_cylinder)
		{
			vFloat3 vR = vP - m_vC;
			vR.z = 0.0;
			float r = vR.r();
			if(r < m_vR.x)bInside = false;
			if(r > m_vR.y)bInside = false;
		}

		IF_F(m_bInside != bInside);
		return true;
	}
};

class _S6H4D: public _ActuatorBase
{
public:
	_S6H4D();
	~_S6H4D();

	virtual bool init(void *pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

protected:
	bool checkArm(void);
	bool checkForbiddenArea(void);

	void updatePos(void);
	void updateSpeed(void);
	void updateRot(void);
	void readState(void);
	void decodeState(void);

	vFloat3 getPtarget(void);
	vFloat3 getP(void);
	vFloat3 getAtarget(void);
	vFloat3 getA(void);

	void gotoPos(vFloat3& vP);
	void stickSpeed(vFloat3 &vM);
	void stickRot(int iAxis, float r);
	void stickStop(void);
	void stickRelease(void);

	void armGotoPos(vFloat3 &vP, vFloat3& vR, float speed);
	void armSetOrigin(vFloat3 &vP);
	void armSetMode(int mode);
	void armPause(void);
	void armReset(void);
	void armCtrlReset(void);

	void update(void);
	static void* getUpdate(void *This)
	{
		((_S6H4D*) This)->update();
		return NULL;
	}

public:
	_IOBase *m_pIO;
	S6H4D_CMD_STATE m_state;

	bool m_bOrder;
	int m_mode;
	vFloat2 m_vSpeedRange;	//mm/m
	float m_speed;
	vFloat3 m_vOriginTarget;
	vFloat3 m_vOrigin;
	vFloat3 m_vLastValidP;

	vFloat3 m_vPgoing;
	float m_pErr;

	vector<S6H4D_VOL> m_vForbArea;
};

}
#endif
