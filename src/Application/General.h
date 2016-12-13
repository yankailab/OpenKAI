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

class General: public AppBase
{
public:
	General();
	~General();

	bool start(Kiss* pKiss);
	void draw(void);
	void handleMouse(int event, int x, int y, int flags);
	void handleKey(int key);
	bool createAllInst(Kiss* pKiss);

private:
	string* getName(void);

public:
	Frame*	m_pFrame;

	Module	m_module;
	int		m_nInst;
	BASE* 	m_pInst[N_INST];

	int 	m_nMouse;
	BASE* 	m_pMouse[N_INST];

	int		m_screenW;
	int		m_screenH;

};

}
#endif
