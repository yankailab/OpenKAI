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

namespace kai
{

	class _StateControl : public _ModuleBase
	{
	public:
		_StateControl();
		virtual ~_StateControl();

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(void) override;
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
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_StateControl *)This)->update();
			return NULL;
		}

	protected:
		vector<StateBase *> m_vpState;
		string m_start = "";
		int m_iS = 0; // current state
	};

}
#endif
