/*
 * _ModuleBase.h
 *
 *  Created on: Feb 2, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__ModuleBase_H_
#define OpenKAI_src_Base__ModuleBase_H_

#include "_Thread.h"

#define ON_PAUSE          \
	if (m_pT->bOnPause()) \
	{                     \
		onPause();        \
	}

#define ON_RESUME          \
	if (m_pT->bOnResume()) \
	{                      \
		onResume();        \
	}

namespace kai
{
	class _ModuleBase : public BASE
	{
	public:
		_ModuleBase();
		virtual ~_ModuleBase();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void));
		virtual bool check(void);
		virtual void console(void *pConsole);

		virtual bool bAlive(void);
		virtual bool bRun(void);
		virtual bool bStop(void);

		virtual void pause(void);
		virtual void resume(void);
		virtual void stop(void);

		virtual void atomicFrom(void);
		virtual void atomicTo(void);

	protected:
		virtual void onPause(void);
		virtual void onResume(void);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ModuleBase *)This)->update();
			return NULL;
		}

	protected:
		_Thread *m_pT;
		pthread_mutex_t m_mutexAtomic;
	};

}
#endif
