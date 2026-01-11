/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_Base_BASE_H_
#define OpenKAI_src_Base_BASE_H_

#include "common.h"
#include "../Module/ModuleMgr.h"

using namespace std;

namespace kai
{
	class BASE
	{
	public:
		BASE();
		virtual ~BASE();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);


		virtual int init(void *pKiss);
		virtual int link(void);

		virtual int start(void);
		virtual int check(void);

		virtual void pause(void);
		virtual void resume(void);
		virtual void stop(void);

		virtual void draw(void *pFrame);
		virtual void console(void *pConsole);

		string getName(void);
		string getClass(void);

	protected:
		string m_class;
		string m_name;


		void *m_pKiss;
		bool m_bLog;
	};

}

#endif
