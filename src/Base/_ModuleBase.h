/*
 * _ModuleBase.h
 *
 *  Created on: Feb 2, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__ModuleBase_H_
#define OpenKAI_src_Base__ModuleBase_H_

#include "_Thread.h"

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

		virtual void onGoSleep(void);
		virtual void onWakeUp(void);

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
