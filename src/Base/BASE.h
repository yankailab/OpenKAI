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
#include "../Utility/utilFile.h"

using namespace std;

namespace kai
{
	class BASE
	{
	public:
		BASE();
		virtual ~BASE();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);

		virtual void pause(void);
		virtual void resume(void);
		virtual void stop(void);

		virtual void draw(void *pFrame);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		virtual bool loadConfig(json *pJ = nullptr, string fName = "");
		virtual bool saveConfig(json &j, string fName = "");

		void setName(const string &n);
		string getName(void);
		string getClass(void);

	protected:
		string m_class;
		string m_name;

		string m_fConfig;

		bool m_bLog;
	};

}

#endif
