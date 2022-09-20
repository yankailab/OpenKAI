/*
 * _ZDmotor.h
 *
 *  Created on: Sept 21, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__ZDmotor_H_
#define OpenKAI_src_Actuator__ZDmotor_H_

#include "../../Protocol/_Modbus.h"
#include "../_ActuatorBase.h"

namespace kai
{

class _ZDmotor: public _ActuatorBase
{
public:
	_ZDmotor();
	~_ZDmotor();

	bool init(void* pKiss);
	bool start(void);

	int check(void);    
    bool power(bool bON);

protected:
	bool setMode(void);
	bool setSpeed(void);
	bool setAccel(void);
	bool setBrake(void);

	bool bComplete(void);
	bool stop(void);
	bool setSlave(int iSlave);
	bool readStatus(void);
    bool clearAlarm(void);
    
    void setup(void);

	void updateMove(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_ZDmotor*) This)->update();
		return NULL;
	}

public:
	_Modbus* m_pMB;
	int		m_iSlave;
	int		m_iMode;
	ACTUATOR_AXIS* m_pA;

	INTERVAL_EVENT m_ieReadStatus;
};

}
#endif
