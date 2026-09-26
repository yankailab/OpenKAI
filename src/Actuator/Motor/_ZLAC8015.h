/*
 * _ZLAC8015.h
 *
 *  Created on: June 22, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__ZLAC8015_H_
#define OpenKAI_src_Actuator__ZLAC8015_H_

#include "../../Protocol/_Modbus.h"
#include "../_ActuatorBase.h"

namespace kai
{

	class _ZLAC8015 : public _ActuatorBase
	{
	public:
		_ZLAC8015();
		~_ZLAC8015();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);

	private:
//		bool setPower(bool bON);
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
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_ZLAC8015 *)This)->update();
			return NULL;
		}

	private:
		_Modbus *m_pMB = nullptr;
		int m_iMode = 3;

		INTERVAL_EVENT m_ieReadStatus;
	};

}
#endif
