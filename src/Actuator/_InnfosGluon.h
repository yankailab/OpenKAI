/*
 * _InnfosGluon.h
 *
 *  Created on: Dec 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__InnfosGluon_H_
#define OpenKAI_src_Actuator__InnfosGluon_H_

#include "_ActuatorBase.h"

#ifdef USE_INNFOS
#include <robot_interface.h>

namespace kai
{

class _InnfosGluon: public _ActuatorBase
{
public:
	_InnfosGluon();
	~_InnfosGluon();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

private:
	void checkAlarm(void);
	void sendCMD(void);
	void readStatus(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_InnfosGluon*) This)->update();
		return NULL;
	}

public:
    ActuatorGroup m_gluon;
    double m_maxLinearAccel;
    double m_maxAngularAccel;
    
	INTERVAL_EVENT m_ieCheckAlarm;
	INTERVAL_EVENT m_ieSendCMD;
	INTERVAL_EVENT m_ieReadStatus;

};

}
#endif
#endif
