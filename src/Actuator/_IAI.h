/*
 * _IAI.h
 *
 *  Created on: Nov 8, 2025
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__IAI_H_
#define OpenKAI_src_Actuator__IAI_H_

#include "../Protocol/_Modbus.h"
#include "_ActuatorBase.h"

namespace kai
{

	class _IAI : public _ActuatorBase
	{
	public:
		_IAI();
		~_IAI();

		int init(void *pKiss);
		int link(void);
		int start(void);
		int check(void);

	protected:
		bool clearAlarm(void);
		bool readStatus(void);

		bool gotoOrigin(void);
		bool setPos(void);
		bool stopMove(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_IAI *)This)->update();
			return NULL;
		}

	protected:
		_Modbus *m_pMB;
	};

}
#endif
