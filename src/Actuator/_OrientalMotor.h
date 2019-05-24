/*
 * _OrientalMotor.h
 *
 *  Created on: May 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__OrientalMotor_H_
#define OpenKAI_src_Actuator__OrientalMotor_H_

#include "../Dependency/libmodbus/modbus.h"
#include "_ActuatorBase.h"

namespace kai
{

class _OrientalMotor: public _ActuatorBase
{
public:
	_OrientalMotor();
	~_OrientalMotor();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);

private:
	bool open(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_OrientalMotor*) This)->update();
		return NULL;
	}

public:
	modbus_t *m_pMb;
	string	m_port;
	int		m_baud;
	int		m_slaveAddr;

};

}
#endif
