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

	void f2b(uint8_t* pB, float f)
	{
	  uint8_t *pchar = (uint8_t*)&f;

	  for(uint8_t i=0; i<sizeof(float); i++)
	  {
	    *pB = *pchar;
	    pchar++;
	    pB++;
	  }
	}

	void set(vFloat3& vP, vFloat3& vR, float spd)
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

struct S6H4D_STATE
{
	uint8_t m_pB[9];
	int m_iB;

	void init(void)
	{
		m_iB = 0;
	}
};

class _S6H4D: public _ActuatorBase
{
public:
	_S6H4D();
	~_S6H4D();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

	void armReset(void);
	void setOrigin(vFloat3& vP);
	void setMode(int mode);
	void move(vFloat3& vM);
	void pause(void);

private:
	void updatePos(void);
	void readState(void);
	void decodeState(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_S6H4D *) This)->update();
		return NULL;
	}

public:
	_IOBase* m_pIO;
	S6H4D_STATE m_state;

	vFloat2	m_vPosRangeX;
	vFloat2	m_vPosRangeY;
	vFloat2	m_vPosRangeZ;

	vFloat2	m_vRotRangeX;
	vFloat2	m_vRotRangeY;
	vFloat2	m_vRotRangeZ;

	vFloat2	m_vSpeedRange;	//mm/m

	//raw values
	vFloat3 m_vP;
	vFloat3 m_vA;
	vFloat2 m_vB;
	float m_aw;
	vFloat3 m_vwkXYZ;
	bool m_bOrder;

};

}
#endif
