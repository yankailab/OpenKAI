/*
 * _StateControl.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai
 */

#ifndef OpenKAI_src_State__StateControl_H_
#define OpenKAI_src_State__StateControl_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"
#include "StateBase.h"

#define ADD_STATE(x)         \
	if (pKs->m_class == #x)  \
	{                        \
		x* pI = new x();     \
		if(!pI->init(pKs))   \
		{					 \
			DEL(pI); 		 \
			LOG_E(pKs->m_name + ": init failed"); \
		}	\
		m_vpState.push_back(pI); \
		continue; \
	} \

namespace kai
{

	class _StateControl : public _ModuleBase
	{
	public:
		_StateControl();
		virtual ~_StateControl();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

		StateBase *getCurrentState(void);
		string getCurrentStateName(void);
		int getCurrentStateIdx(void);
		STATE_TYPE getCurrentStateType(void);

		int getStateIdxByName(const string &n);
		void transit(void);
		void transit(const string &n);
		void transit(int iS);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_StateControl *)This)->update();
			return NULL;
		}

		vector<StateBase *> m_vpState;
		int m_iS; // current state
	};

}
#endif
