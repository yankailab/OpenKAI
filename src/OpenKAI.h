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
	class OpenKAI
	{
	public:
		OpenKAI();
		~OpenKAI();

		// system
		bool init(const string& appName = "OpenKAI");
		void clean(void);

		// module create
		bool addKiss(const string &fName);
		bool addModule(void* pModule, const string& mName);
		void* findModule(const string& mName);
		bool deleteModule(const string& mName);

		// general flow
		bool createAllModules(void);
		int initAllModules(void);
		int linkAllModules(void);
		int startAllModules(void);
		void resumeAllModules(void);
		void pauseAllModules(void);
		void stopAllModules(void);
		void waitForComplete(void);
		bool bComplete(void);

		// access methods
		void setName(const string &n);
		string getName(void);
		void printEnvironment(void);

	private:
		int check(void);

	private:
		void *m_pKiss;

		string m_appName;
		bool m_bRun;
		bool m_bStdErr;
		bool m_bLog;
		string m_rc;
	};

}
#endif
