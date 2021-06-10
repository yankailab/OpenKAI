/*
 * _HYMCU_RS485.h
 *
 *  Created on: June 22, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__DRV8825_RS485_H_
#define OpenKAI_src_Actuator__DRV8825_RS485_H_

#include "../Protocol/_Modbus.h"
#include "_ActuatorBase.h"

namespace kai
{

struct HYMCU_MODBUS_ADDR
{
	//default for DRV8825
	uint16_t m_setDPR = 4;
	uint16_t m_setDist = 9;
	uint16_t m_setDir = 11;
	uint16_t m_setSpd = 8;
	uint16_t m_setAcc = 2;
	uint16_t m_setSlaveID = 16;

	uint16_t m_bComplete = 12;
	uint16_t m_readStat = 22;

	uint16_t m_run = 7;
	uint16_t m_stop = 3;
	uint16_t m_resPos = 10;
	uint16_t m_saveData = 0;
};

class _HYMCU_RS485: public _ActuatorBase
{
public:
	_HYMCU_RS485();
	~_HYMCU_RS485();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

protected:
	bool setSlaveID(uint16_t iSlave);
	bool saveData(void);

	bool setDistPerRound(int32_t dpr);
	bool setPos(void);
	bool setSpeed(void);
	bool setAccel(void);
	bool setBrake(void);
	bool resetPos(void);
	bool bComplete(void);

	bool run(void);
	bool stop(void);
	bool initPos(void);

	bool readStatus(void);
	void updateMove(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_HYMCU_RS485*) This)->update();
		return NULL;
	}

public:
	_Modbus* m_pMB;
	int		m_iSlave;
	int		m_iData;
	int32_t m_dpr;	//distance per round
	ACTUATOR_AXIS* m_pA;

	HYMCU_MODBUS_ADDR m_addr;
	int32_t m_dInit;
	uint32_t m_cmdInt;

	INTERVAL_EVENT m_ieReadStatus;

};

}
#endif
