#ifndef OpenKAI_src_Module_ModuleMgr_H_
#define OpenKAI_src_Module_ModuleMgr_H_

#include "JsonCfg.h"

namespace kai
{
	class BASE;

	class ModuleMgr
	{
	public:
		ModuleMgr(void);
		~ModuleMgr(void);

		bool loadJsonFiles(const string &fName);
		JsonCfg* findJsonCfg(const string &name);

		bool createAll(void);
		bool initAll(void);
		bool linkAll(void);
		bool startAll(void);
		void pauseAll(void);
		void resumeAll(void);
		void stopAll(void);

		void waitForComplete(void);
		bool bComplete(void);
		void cleanAll(void);

		void *findModule(const string &name);
		json* findJson(const string &name);

		bool addModule(void *pModule, const string &name);

		bool bStdErr(void);
		string getName(void);

	protected:
		string m_name = "ModuleMgr";
		bool m_bLog = true;

		vector<JsonCfg> m_vJcfg;	// correspondent to each .json file
		vector<BASE *> m_vModules;
		json m_jNull = nullptr;
	};

}
#endif
