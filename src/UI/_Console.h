/*
 * _Console.h
 *
 *  Created on: Oct 13, 2019
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI__Console_H_
#define OpenKAI_src_UI__Console_H_

#include "../Base/_ModuleBase.h"

namespace kai
{

	class _Console : public _ModuleBase
	{
	public:
		_Console();
		virtual ~_Console();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);

		void addMsg(const string &msg, int iLine = 1);
		void addMsg(const string &msg, int iCol, int iX, int iLine = 0);

	protected:
		void updateConsole(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Console *)This)->update();
			return NULL;
		}

	private:
		vector<BASE *> m_vpB;
		int m_iY;
	};

}
#endif
