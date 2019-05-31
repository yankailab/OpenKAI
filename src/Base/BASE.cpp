/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#include "BASE.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"

namespace kai
{

BASE::BASE()
{
	m_pKiss = NULL;
	m_pWindow = NULL;
	m_bLog = false;
	m_bDraw = true;

	m_consoleMsg = "";
	m_consoleMsgLevel = -1;
}

BASE::~BASE()
{
}

bool BASE::init(void* pKiss)
{
	NULL_F(pKiss);
	Kiss* pK = (Kiss*)pKiss;
	m_pKiss = pKiss;

	string name="";
	F_ERROR_F(pK->v("name",&name));
	IF_F(name.empty());

	bool bLog = false;
	pK->root()->o("APP")->v("bLog",&bLog);
	if(bLog)
	{
		pK->v("bLog",&m_bLog);
	}

	pK->v("bDraw",&m_bDraw);

	name = "";
	F_INFO(pK->v("Window",&name));
	m_pWindow = (Window*)(pK->root()->getChildInst(name));

	return true;
}

string* BASE::getName(void)
{
	NULL_N(m_pKiss);
	return &((Kiss*)m_pKiss)->m_name;
}

string* BASE::getClass(void)
{
	NULL_N(m_pKiss);
	return &((Kiss*)m_pKiss)->m_class;
}

bool BASE::start(void)
{
	return true;
}

int BASE::check(void)
{
	return -1;
}

int BASE::serialize(uint8_t* pB, int nB)
{
	return 0;
}

int BASE::deSerialize(uint8_t* pB, int nB)
{
	return 0;
}

bool BASE::draw(void)
{
	IF_F(!m_bDraw);
	NULL_F(m_pWindow);

	Window* pWin = (Window*)m_pWindow;
	NULL_F(pWin->getFrame());

	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	return true;
}

bool BASE::console(int& iY)
{
    string msg;
    C_NAME(*this->getName());

    if(m_consoleMsgLevel > 0)
    	COL_ERROR;
    else
    	COL_MSG;

    C_MSG(m_consoleMsg);

    return true;
}

}
