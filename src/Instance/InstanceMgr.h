#ifndef OpenKAI_src_Instance_InstanceMgr_H_
#define OpenKAI_src_Instance_InstanceMgr_H_

#include "../DataStream/DataStreamBase.h"
#include "JsonCfg.h"

namespace kai
{
	class _ModuleBase;

	class InstanceMgr
	{
	public:
		InstanceMgr(void);
		~InstanceMgr(void);

		bool loadJsonFiles(const string &fName);

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

		bool addModule(const string& name, JsonCfg* pJc, json* pJ);
		bool addDataStream(const string& name, JsonCfg* pJc, json* pJ);

		void *findModule(const string &name);
		void *findDataStream(const string &name);
		JsonCfg *findJsonCfg(const string &name);
		json *findJson(const string &name);

		bool addExternalModule(void *pModule, const string &name);

		bool bStdErr(void);
		string getName(void);

	protected:
		string m_name = "InstanceMgr";
		bool m_bLog = true;

		vector<JsonCfg> m_vJcfg;		  // correspondent to each .json file
		vector<_ModuleBase *> m_vModules; // hold flat from all the .json files
		vector<DataStreamBase *> m_vDataStreams;
	};

}
#endif
