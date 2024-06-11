/*
 * OpenKAI.h
 *
 *  Created on: Aug 20, 2015
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_OpenKAI_H_
#define OpenKAI_src_OpenKAI_H_

#include <string>
#include <vector>

using namespace std;

namespace kai
{

	struct OK_MODULE
	{
		void *m_pInst;
		void *m_pKiss;
	};

	class OpenKAI
	{
	public:
		OpenKAI();
		~OpenKAI();

		bool init(void);
		bool addModulesFromKiss(const string &fName);
		bool createAllModules(void);
		bool initAllModules(void);
		bool linkAllModules(void);
		bool startAllModules(void);
		void runAllModules(void);
		void pauseAllModules(void);
		void stopAllModules(void);
		void waitForComplete(void);
		bool bComplete(void);
		void exit(void);

		void setName(const string &n);
		string getName(void);

		void printEnvironment(void);

	protected:
		int check(void);

	protected:
		vector<OK_MODULE> m_vInst;
		void *m_pKiss;

		string m_appName;
		bool m_bRun;
		bool m_bStdErr;
		bool m_bLog;
		string m_rc;
	};

}
#endif
