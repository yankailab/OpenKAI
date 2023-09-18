/*
 * _ThreadCtrl.h
 *
 *  Created on: Aug 2, 2023
 *      Author: Kai
 */

#ifndef OpenKAI_src_Base__ThreadCtrl_H_
#define OpenKAI_src_Base__ThreadCtrl_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"

namespace kai
{

	struct STATE_THREAD
	{
		string m_name = "";
		int m_iState = -1;
		vector<_Thread *> m_vpT;

		bool bState(const string &n)
		{
			return (n == m_name);
		}

		bool bState(int i)
		{
			return (i == m_iState);
		}

		void addThread(_Thread *pT)
		{
			m_vpT.push_back(pT);
		}

		void update(int s)
		{
			if (s == m_iState)
			{
				for (_Thread *pT : m_vpT)
					pT->wakeUp();
			}
			else
			{
				for (_Thread *pT : m_vpT)
					pT->goSleep();
			}
		}
	};

	class _ThreadCtrl : public _ModuleBase
	{
	public:
		_ThreadCtrl();
		virtual ~_ThreadCtrl();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual void console(void *pConsole);

		string getCurrentStateName(void);
		int getCurrentStateIdx(void);
		int getStateIdxByName(const string &n);
		void transitTo(const string &n);
		void transitTo(int iS);

	private:
		void updateStates(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ThreadCtrl *)This)->update();
			return NULL;
		}

		vector<STATE_THREAD> m_vStates;
		int m_iS; // current state
		int m_iTransit;
	};

}
#endif
