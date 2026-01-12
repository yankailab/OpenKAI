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

	class _ZDmotor : public _ActuatorBase
	{
	public:
		_ZDmotor();
		~_ZDmotor();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);

	protected:
		bool setPower(bool bON);
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

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ZDmotor *)This)->update();
			return NULL;
		}

	protected:
		_Modbus *m_pMB;
		int m_iMode;
	};

}
#endif
