/*
 * OpenKAI.h
 *
 *  Created on: Aug 20, 2015
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_OpenKAI_H_
#define OpenKAI_src_OpenKAI_H_

// #include <string>
// #include <vector>
#include "Base/common.h"

using namespace std;

namespace kai
{
	class OpenKAI
	{
	public:
		OpenKAI();
		~OpenKAI();

		// system
		int init(const string& appName = "OpenKAI");
		void clean(void);

		// module create
		int addKiss(const string &fName);
		int addModule(void* pModule, const string& mName);
		void* findModule(const string& mName);
		void* findModuleKiss(const string& mName);
		int deleteModule(const string& mName);

		// general flow
		int createAllModules(void);
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
		void logEnvironment(void);

	private:
		bool check(void);

	private:
		void *m_pKiss;

		string m_appName;
		bool m_bStdErr;
		string m_rc;

		bool m_bLog = 1;
	};

}
#endif
