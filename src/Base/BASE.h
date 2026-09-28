/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_Base_BASE_H_
#define OpenKAI_src_Base_BASE_H_

#include "common.h"
#include "../Module/JsonCfg.h"

using namespace std;

namespace kai
{
	class BASE
	{
	public:
		BASE();
		virtual ~BASE();

		void setConfig(JsonCfg *pJcfg, json *pJ);
		void setName(const string &n);
		string getName(void);
		string getClass(void);

		virtual bool loadConfig(void);
		virtual bool saveConfig(bool bExport = false);

		virtual void draw(void *pMat);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

	protected:
		JsonCfg *m_pJcfg = nullptr;
		json *m_pJ = nullptr;

		string m_class = "";
		string m_name = "";

		bool m_bLog = false;
	};

}

#endif
