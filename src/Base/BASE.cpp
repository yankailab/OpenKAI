/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#include "BASE.h"
#include "../Script/Kiss.h"
#include "../UI/_Console.h"

namespace kai
{

	BASE::BASE()
	{
		m_pKiss = NULL;
		m_bLog = false;
		m_bDebug = false;
	}

	BASE::~BASE()
	{
	}

	bool BASE::init(void *pKiss)
	{
		NULL_F(pKiss);
		Kiss *pK = (Kiss *)pKiss;
		m_pKiss = pKiss;

		pK->v("bLog", &m_bLog);
		pK->v("bDebug", &m_bDebug);

		return true;
	}

	bool BASE::link(void)
	{
		NULL_F(m_pKiss);

		return true;
	}

	string *BASE::getName(void)
	{
		NULL_N(m_pKiss);
		return &((Kiss *)m_pKiss)->m_name;
	}

	string *BASE::getClass(void)
	{
		NULL_N(m_pKiss);
		return &((Kiss *)m_pKiss)->m_class;
	}

	bool BASE::start(void)
	{
		return true;
	}

	int BASE::check(void)
	{
		return 0;
	}

	int BASE::serialize(uint8_t *pB, int nB)
	{
		return 0;
	}

	int BASE::deSerialize(uint8_t *pB, int nB)
	{
		return 0;
	}

	void BASE::draw(void *pFrame)
	{
	}

	void BASE::console(void *pConsole)
	{
		NULL_(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("____________________________________", COLOR_PAIR(_Console_COL_NAME) | A_BOLD, _Console_X_NAME, 1);
		pC->addMsg(*this->getName(), COLOR_PAIR(_Console_COL_NAME) | A_BOLD, _Console_X_NAME, 1);
	}

	void BASE::context(void* pContext)
	{
	}

}
