/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#include "BASE.h"
#include "../Script/Kiss.h"
#include "../UI/Window.h"
#include "../UI/Console.h"

namespace kai
{

BASE::BASE()
{
	m_pKiss = NULL;
	m_pWindow = NULL;
	m_pConsole = NULL;
	m_bLog = false;
	m_bDebug = false;
}

BASE::~BASE()
{
}

bool BASE::init(void* pKiss)
{
	NULL_F(pKiss);
	Kiss* pK = (Kiss*)pKiss;
	m_pKiss = pKiss;

	string n="";
	F_ERROR_F(pK->v("name",&n ));
	IF_F( n.empty());

	bool bLog = false;
	pK->root()->child("APP")->v("bLog",&bLog);
	if(bLog)
	{
		pK->v("bLog",&m_bLog);
	}
	pK->v("bDebug",&m_bDebug);

	n = "";
	pK->v("Console",&n );
	m_pConsole = (Console*)(pK->getInst( n ));

#ifdef USE_OPENCV
	n = "";
	pK->v("Window",&n );
	m_pWindow = (Window*)(pK->getInst( n ));
#endif

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

bool BASE::checkWindow(void)
{
	NULL_F(m_pWindow);

#ifdef USE_OPENCV
	Window* pWin = (Window*)m_pWindow;
	NULL_F(pWin->getFrame());

	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	return true;
#else
	return false;
#endif

}

void BASE::draw(void)
{
	if(m_pConsole)
	{
		Console* pC = (Console*)m_pConsole;
		pC->addMsg(*this->getName(), COLOR_PAIR(CONSOLE_COL_NAME)|A_BOLD, CONSOLE_X_NAME, 1);

//	    if(m_msgLev > 0)
//			pC->addMsg(m_msg, COLOR_PAIR(CONSOLE_COL_ERROR)|A_BOLD, CONSOLE_X_MSG);
//	    else
//			pC->addMsg(m_msg, COLOR_PAIR(CONSOLE_COL_MSG), CONSOLE_X_MSG);
	}

#ifdef USE_OPENCV
	if(checkWindow())
	{
		Window* pWin = (Window*)this->m_pWindow;
		pWin->addMsg(*this->getName());
	}
#endif
}

void BASE::addMsg(const string& msg, int iTab)
{
	if(m_pConsole)
		((Console*)m_pConsole)->addMsg(msg, COLOR_PAIR(CONSOLE_COL_MSG), CONSOLE_X_MSG, 1);

#ifdef USE_OPENCV
	if(checkWindow())
	{
		Window* pWin = ((Window*)m_pWindow);
		pWin->addMsg(msg, iTab);
	}
#endif
}

}
