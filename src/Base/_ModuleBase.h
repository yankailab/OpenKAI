/*
 * _ModuleBase.h
 *
 *  Created on: Feb 2, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__ModuleBase_H_
#define OpenKAI_src_Base__ModuleBase_H_

#include "_Thread.h"
#include "../IPC/SharedMem.h"

#define ON_SLEEP            \
	if (m_pT->bonSleep()) \
	{                       \
		onSleep();        \
	}
	
#define ON_WAKEUP          \
	if (m_pT->bOnWakeUp()) \
	{                      \
		onWakeUp();        \
	}

namespace kai
{

	class _ModuleBase : public BASE
	{
	public:
		_ModuleBase();
		virtual ~_ModuleBase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		virtual void sleep(void);
		virtual void run(void);

	protected:
		virtual void onWakeUp(void);
		virtual void onSleep(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ModuleBase *)This)->update();
			return NULL;
		}

	protected:
		_Thread *m_pT;
	};

}
#endif
