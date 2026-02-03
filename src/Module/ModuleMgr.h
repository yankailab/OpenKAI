#ifndef OpenKAI_src_Module_ModuleMgr_H_
#define OpenKAI_src_Module_ModuleMgr_H_

#include "JsonCfg.h"

namespace kai
{

	class ModuleMgr
	{
	public:
		ModuleMgr(void);
		~ModuleMgr(void);

		bool parseJsonFile(const string &fName);

		bool createAll(void);
		bool initAll(void);
		bool linkAll(void);
		bool startAll(void);
		void resumeAll(void);
		void pauseAll(void);
		void stopAll(void);
		void waitForComplete(void);
		bool bComplete(void);
		void cleanAll(void);

		void *findModule(const string &name);
		const json& findJson(const string &name);
		bool addModule(void *pModule, const string &name);

		string getName(void);

	protected:
		string m_name;
		bool m_bLog;

		vector<JsonCfg> m_vJcfg;
		vector<void *> m_vModules;
		json m_jNull;
	};

}
#endif
