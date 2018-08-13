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

class Startup
{
public:
	Startup();
	~Startup();

	bool start(Kiss* pKiss);
	void draw(void);
	void cli(void);
	void handleKey(int key);
	bool createAllInst(Kiss* pKiss);

private:
	string* getName(void);

public:
	Module	m_module;
	int		m_nInst;
	BASE* 	m_ppInst[N_INST];

	string	m_appName;
	int		m_key;
	bool	m_bRun;
	bool	m_bWindow;
	bool	m_bDraw;
	int		m_waitKey;
	bool	m_bLog;
	bool	m_bStdErr;
	string	m_rc;

	string	m_cliMsg;
	int8_t m_cliMsgLevel;
};

}
#endif
