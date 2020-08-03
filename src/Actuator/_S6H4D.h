#ifndef OpenKAI_src_Actuator__S6H4D_H_
#define OpenKAI_src_Actuator__S6H4D_H_

#include "_ActuatorBase.h"
#include "../IO/_IOBase.h"

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

struct S6H4D_AREA
{
	vFloat2 m_vX;
	vFloat2 m_vY;
	vFloat2 m_vZ;

	void init(void)
	{
		m_vX.init(0.0);
		m_vY.init(0.0);
		m_vZ.init(0.0);
	}

	bool bInside(vFloat3& vP)
	{
		IF_F((m_vX.x != 0.0) && (vP.x < m_vX.x));
		IF_F((m_vX.y != 0.0) && (vP.x > m_vX.y));
		IF_F((m_vY.x != 0.0) && (vP.y < m_vY.x));
		IF_F((m_vY.y != 0.0) && (vP.y > m_vY.y));
		IF_F((m_vZ.x != 0.0) && (vP.z < m_vZ.x));
		IF_F((m_vZ.y != 0.0) && (vP.z > m_vZ.y));

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

	void armReset(void);
	void setOrigin(vFloat3 &vP);
	void setMode(int mode);
	void pause(void);
	void resetCtrl(void);

	void gotoPos(vFloat3 &vP, vFloat3& vR, float speed);
	void move(vFloat3 &vM);
	void rot(int iAxis, float r);

private:
	bool checkRange(void);
	void updatePos(void);
	void updateMove(void);
	void updateRot(void);
	void autoGripperHdg(void);
	void readState(void);
	void decodeState(void);
	void update(void);
	static void* getUpdateThread(void *This)
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

	vFloat2 m_vRrange;
	vector<S6H4D_AREA> m_vForbArea;

};

}
#endif
