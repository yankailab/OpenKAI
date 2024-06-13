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
		void *m_pBase;
		void *m_pKiss;
	};

	class OpenKAI
	{
	public:
		OpenKAI();
		~OpenKAI();

		// system
		bool init(const string& appName = "OpenKAI");

		// module create
		bool addKiss(const string &fName);
		int addModule(void* pModule, const string& mName);

		// general flow
		bool createAllModules(void);
		bool initAllModules(void);
		bool linkAllModules(void);
		bool startAllModules(void);
		void resumeAllModules(void);
		void pauseAllModules(void);
		void stopAllModules(void);
		void waitForComplete(void);
		bool bComplete(void);
		void exit(void);

		// module control
		void* getModule(const string& mName);
		int getModuleHandle(const string& mName);
		bool startModule(int h);
		bool pauseModule(int h);
		bool resumeModule(int h);
		bool stopModule(int h);
		bool deleteModule(int h);

		// access methods
		void setName(const string &n);
		string getName(void);
		void printEnvironment(void);

	protected:
		int check(void);
		int findModule(const string& mName);

	protected:
		vector<OK_MODULE> m_vModules;
		void *m_pKiss;

		string m_appName;
		bool m_bRun;
		bool m_bStdErr;
		bool m_bLog;
		string m_rc;
	};

}
#endif
