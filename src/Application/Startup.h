/*
 * General.h
 *
 *  Created on: Aug 20, 2015
 *      Author: yankai
 */

#ifndef SRC_GENERAL_H_
#define SRC_GENERAL_H_

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
	void handleMouse(int event, int x, int y, int flags);
	void handleKey(int key);
	bool createAllInst(Kiss* pKiss);

private:
	string* getName(void);

public:
	Module	m_module;
	int		m_nInst;
	BASE* 	m_pInst[N_INST];

	int 	m_nMouse;
	BASE* 	m_pMouse[N_INST];

	string m_name;
	int m_key;
	bool m_bRun;
	bool m_bWindow;
	int m_waitKey;
	bool m_bLog;
};

}
#endif
