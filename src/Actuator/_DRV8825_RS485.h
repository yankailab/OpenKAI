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

	bool setDistPerRound(int32_t dpr);
	bool setDist(int32_t d);
	bool setSpeed(void);
	bool setAccel(void);
	bool setBrake(void);
	void resetPos(void);

	void run(void);
	void stop(void);
	void reset(void);

protected:
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

	INTERVAL_EVENT m_ieReadStatus;

};

}
#endif
