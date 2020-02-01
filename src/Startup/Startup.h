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

using namespace kai;

namespace kai
{

typedef void (*CallbackKey)(int key, void* pfInst);

struct KEY_CALLBACK
{
	CallbackKey m_cbKey;
	void* m_pInst;
};

struct OK_INST
{
	BASE* m_pInst;
	Kiss* m_pKiss;
};

class Startup
{
public:
	Startup();
	~Startup();

	bool start(Kiss* pKiss);
	void handleKey(int key);
	bool createAllInst(Kiss* pKiss);
	void printEnvironment(void);
	bool addKeyCallback(CallbackKey cbKey, void* pInst);

private:
	string* getName(void);

public:
	Module	m_module;
	vector<OK_INST> m_vInst;

	string	m_appName;
	int		m_key;
	bool	m_bRun;
	bool	m_bWindow;
	bool	m_bDraw;
	int		m_waitKey;
	bool	m_bLog;
	bool	m_bStdErr;
	string	m_rc;

	vector<KEY_CALLBACK> m_vKeyCallback;
};

}
#endif
