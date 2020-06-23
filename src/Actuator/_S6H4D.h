#ifndef OpenKAI_src_Actuator__S6H4D_H_
#define OpenKAI_src_Actuator__S6H4D_H_

#include "_ActuatorBase.h"
#include "../IO/_IOBase.h"

#define S6H4D_BEGIN 0xee
#define S6H4D_END 0xef

namespace kai
{

struct S6H4D_CMD
{
	uint8_t m_begin;
	uint8_t m_cmd1;
	uint8_t m_cmd2;
	float m_pV[11];
	uint8_t m_end;

	void init(void)
	{
		m_begin = S6H4D_BEGIN;
		m_cmd1 = 0;
		m_cmd2 = 0;

		for(int i=0; i<11; i++)
			m_pV[i] = 0;

		m_end = S6H4D_END;
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

	vFloat3 m_vPos;
	vFloat2	m_vPosRangeX;
	vFloat2	m_vPosRangeY;
	vFloat2	m_vPosRangeZ;

	vFloat3 m_vRot;
	vFloat2	m_vRotRangeX;
	vFloat2	m_vRotRangeY;
	vFloat2	m_vRotRangeZ;

	vFloat2	m_vSpeedRange;	//mm/m

};

}
#endif
