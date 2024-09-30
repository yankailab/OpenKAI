/*
 * _ZLAC8015D.h
 *
 *  Created on: June 22, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__ZLAC8015D_H_
#define OpenKAI_src_Actuator__ZLAC8015D_H_

#include "../../Protocol/_Modbus.h"
#include "../_ActuatorBase.h"

namespace kai
{

	class _ZLAC8015D : public _ActuatorBase
	{
	public:
		_ZLAC8015D();
		~_ZLAC8015D();

		int init(void *pKiss);
		int link(void);
		int start(void);
		int check(void);
		
	private:
		bool setMode(void);
		bool setSpeed(void);
		bool setAccel(void);
		bool setBrake(void);

		bool bComplete(void);
		bool stopMove(void);
		bool setSlave(int iSlave);
		bool readStatus(void);
		bool clearAlarm(void);

		void setup(void);

		void updateMove(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ZLAC8015D *)This)->update();
			return NULL;
		}

	private:
		_Modbus *m_pMB;
		int m_iSlave;
		int m_iMode;

		INTERVAL_EVENT m_ieReadStatus;
	};

}
#endif
