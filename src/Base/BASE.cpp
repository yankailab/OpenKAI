/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#include "BASE.h"
#include "../Module/Kiss.h"
#include "../UI/_Console.h"

namespace kai
{

	BASE::BASE()
	{
		m_pKiss = NULL;
		m_bLog = false;
	}

	BASE::~BASE()
	{
	}

	bool BASE::init(void *pKiss)
	{
		NULL_F(pKiss);

		m_pKiss = pKiss;
		Kiss *pK = (Kiss *)pKiss;
		pK->setModule(this);

		pK->v("bLog", &m_bLog);

		return true;
	}

	bool BASE::link(void)
	{
		NULL_F(m_pKiss);

		return true;
	}

	string BASE::getName(void)
	{
		if(!m_pKiss)
			return "";

		return ((Kiss *)m_pKiss)->getName();
	}

	string BASE::getClass(void)
	{
		if(!m_pKiss)
			return "";

		return ((Kiss *)m_pKiss)->getClass();
	}

	bool BASE::start(void)
	{
		return true;
	}

    void BASE::pause(void)
    {
    }

    void BASE::resume(void)
    {
    }

    void BASE::stop(void)
    {
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
		pC->addMsg(this->getName(), COLOR_PAIR(_Console_COL_NAME) | A_BOLD, _Console_X_NAME, 1);
	}

	void BASE::context(void* pContext)
	{
	}

}
