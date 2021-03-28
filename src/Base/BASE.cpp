/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#include "BASE.h"
#include "../Script/Kiss.h"
#include "../UI/_WindowCV.h"
#include "../UI/_Console.h"

namespace kai
{

BASE::BASE()
{
	m_pKiss = NULL;
	m_pWindow = NULL;
	m_p_Console = NULL;
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
	pK->v("_Console",&n );
	m_p_Console = (_Console*)(pK->getInst( n ));

#ifdef USE_OPENCV
	n = "";
	pK->v("Window",&n );
	m_pWindow = (_WindowCV*)(pK->getInst( n ));
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
	return 0;
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
	_WindowCV* pWin = (_WindowCV*)m_pWindow;
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
	if(m_p_Console)
	{
		_Console* pC = (_Console*)m_p_Console;
		pC->addMsg("____________________________________", COLOR_PAIR(_Console_COL_NAME)|A_BOLD, _Console_X_NAME, 1);
		pC->addMsg(*this->getName(), COLOR_PAIR(_Console_COL_NAME)|A_BOLD, _Console_X_NAME, 1);
	}
}

void BASE::addMsg(const string& msg, int iLine)
{
	NULL_(m_p_Console);
    
	((_Console*)m_p_Console)->addMsg(msg, COLOR_PAIR(_Console_COL_MSG), _Console_X_MSG, iLine);
}

}
