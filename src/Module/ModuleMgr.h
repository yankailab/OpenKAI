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
		void setJsonCfg(const JsonCfg &jCfg);
		string getName(void);

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
		bool addModule(void *pModule, const string &name);


	protected:
		string m_name;
		bool m_bLog;

		JsonCfg m_jCfg;
		vector<void *> m_vModules;
	};

}
#endif
