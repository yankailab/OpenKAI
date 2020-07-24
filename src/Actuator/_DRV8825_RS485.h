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
	bool setDist(float d);
	bool setSpeed(float s);
	bool setAccel(float a);
	bool setBrake(float b);
	void run(void);
	void stop(void);

protected:
	void readStatus(void);
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

	vInt2	m_vStepRange;
	vInt2	m_vSpeedRange;	//rpm
	vInt2	m_vAccelRange;
	vInt2	m_vBrakeRange;
	vInt2	m_vCurrentRange;

	INTERVAL_EVENT m_ieReadStatus;

};

}
#endif
