/*
 * _DRV8825_RS485.h
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

class _DRV8825_RS485: public _ActuatorBase
{
public:
	_DRV8825_RS485();
	~_DRV8825_RS485();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

protected:
	bool setDistPerRound(int32_t dpr);
	bool setPos(void);
	bool setSpeed(void);
	bool setAccel(void);
	bool setBrake(void);
	void resetPos(void);
	bool bComplete(void);

	void run(void);
	void stop(void);
	bool initPos(void);

	void readStatus(void);
	void updateMove(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DRV8825_RS485*) This)->update();
		return NULL;
	}

public:
	_Modbus* m_pMB;
	int		m_iSlave;
	int		m_iData;
	int32_t m_dpr;	//distance per round
	ACTUATOR_AXIS* m_pA;

	int32_t m_dInit;

	INTERVAL_EVENT m_ieReadStatus;

};

}
#endif
