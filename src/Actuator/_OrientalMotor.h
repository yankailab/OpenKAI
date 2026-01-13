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
	class _OrientalMotor : public _ActuatorBase
	{
	public:
		_OrientalMotor();
		~_OrientalMotor();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);

	protected:
		bool clearAlarm(void);
		bool readStatus(void);

		bool setPos(void);
		bool setSpeed(void);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_OrientalMotor *)This)->update();
			return NULL;
		}

	protected:
		_Modbus *m_pMB;
		int m_iData;
	};

}
#endif
