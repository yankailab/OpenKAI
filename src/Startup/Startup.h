/*
 * Startup.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Startup_Startup_H_
#define OpenKAI_src_Startup_Startup_H_

#include "../Base/common.h"
#include "../Config/Module.h"
#include "../Utility/utilFile.h"

using namespace kai;

namespace kai
{

	struct OK_INST
	{
		BASE *m_pInst;
		Kiss *m_pKiss;
	};

	class Startup
	{
	public:
		Startup();
		~Startup();

		bool start(const string& fName);
		bool createAllInst(Kiss *pKiss);
		void printEnvironment(void);

	private:
		string *getName(void);

	public:
		Module m_module;
		vector<OK_INST> m_vInst;

		string m_appName;
		bool m_bRun;
		bool m_bStdErr;
		bool m_bLog;
		string m_rc;
	};

	static Startup *g_pStartup;

}
#endif
