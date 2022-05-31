/*
 * _RaspiWSbattery.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_Measurement__RaspiWSbattery_H_
#define OpenKAI_src_Application_Measurement__RaspiWSbattery_H_

#include "../../Base/_ModuleBase.h"
#include "../../Vision/_Frame.h"

namespace kai
{
	class _RaspiWSbattery : public _ModuleBase
	{
	public:
		_RaspiWSbattery(void);
		virtual ~_RaspiWSbattery();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void console(void *pConsole);
		void draw(void *pFrame);

	protected:
		bool updateBatt(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_RaspiWSbattery *)This)->update();
			return NULL;
		}

	private:
		string m_cmdBatt;
		float m_battV;	// voltage
		float m_battA;	// current
		float m_battW;	// power
		float m_battP;	// percent
		float m_battShutdown;
	};

}
#endif
