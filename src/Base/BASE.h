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

		void setModuleMgr(ModuleMgr *pM);
		// Embedded objects bind to a node owned by the parent module's JsonCfg.
		void setConfig(JsonCfg *pJcfg, json *pJ);
		void setName(const string &n);
		string getName(void);
		string getClass(void);

		virtual bool loadConfig(void);
		virtual bool saveConfig(void);

		virtual bool link(void);
		virtual bool start(void);
		virtual bool check(void);

		virtual void pause(void);
		virtual void resume(void);
		virtual void stop(void);

		virtual void draw(void *pMat);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

	protected:
		ModuleMgr *m_pM = nullptr;
		JsonCfg *m_pJcfg = nullptr;
		json *m_pJ = nullptr;

		string m_class = "";
		string m_name = "";

		bool m_bLog = false;
	};

}

#endif
