/*
 * _StepperGripper.h
 *
 *  Created on: June 22, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__StepperGripper_H_
#define OpenKAI_src_Actuator__StepperGripper_H_

#include "_HYMCU_RS485.h"

namespace kai
{

	class _StepperGripper : public _HYMCU_RS485
	{
	public:
		_StepperGripper();
		~_StepperGripper();

		bool init(void *pKiss);
		bool start(void);
		void console(void *pConsole);

		void grip(bool bOpen);
		bool bGrip(void);

	private:
		bool setMove(bool bOpen);
		void updateMove(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_StepperGripper *)This)->update();
			return NULL;
		}

	public:
		bool m_bState; // true:open false:close
		bool m_bOpen;

		float m_pOpen;
		float m_pClose;
	};

}
#endif
