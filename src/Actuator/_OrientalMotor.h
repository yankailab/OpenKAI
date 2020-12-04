/*
 * _OrientalMotor.h
 *
 *  Created on: May 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__OrientalMotor_H_
#define OpenKAI_src_Actuator__OrientalMotor_H_

#include "../Protocol/_Modbus.h"
#include "_ActuatorBase.h"

namespace kai
{

struct OM_STATE
{
	int32_t	m_step;		//position
	int32_t	m_speed;	//Hz
	int32_t	m_accel;	//0.001kHz/s
	int32_t	m_brake;	//0.001kHz/s
	int32_t	m_current;	//0.1%

	void init(void)
	{
		m_step = 0;
		m_speed = 1e3;
		m_accel = 1e6;
		m_brake = 1e6;
		m_current = 1e3;
	}
};

class _OrientalMotor: public _ActuatorBase
{
public:
	_OrientalMotor();
	~_OrientalMotor();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

private:
	void checkAlarm(void);
	void updatePos (void);
	void updateSpeed (void);
	void readStatus(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_OrientalMotor*) This)->update();
		return NULL;
	}

public:
	ACTUATOR_AXIS* m_pA;
	_Modbus* m_pMB;
	int		m_iSlave;
	int		m_iData;

	INTERVAL_EVENT m_ieCheckAlarm;
	INTERVAL_EVENT m_ieSendCMD;
	INTERVAL_EVENT m_ieReadStatus;

};

}
#endif
