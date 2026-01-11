#ifndef OpenKAI_src_Module_ModuleMgr_H_
#define OpenKAI_src_Module_ModuleMgr_H_

#include "../Base/BASE.h"
#include "Module.h"
#include "JsonCfg.h"

namespace kai
{

	class ModuleMgr
	{
	public:
		ModuleMgr(void);
		~ModuleMgr(void);

		bool addJsonCfg(JsonCfg* pJc);
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

		void clean(void);

		void *findModule(const string &name);


	private:
		string m_name;
		bool m_bLog;

		JsonCfg* m_pJcfg;
		vector<BASE*> m_vModules;
	};

}
#endif
