/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#include "BASE.h"
#include "../UI/_Console.h"

#include "../Module/Kiss.h"


namespace kai
{

	BASE::BASE()
	{
		m_pKiss = nullptr;
	}

	BASE::~BASE()
	{
	}

	bool BASE::init(const json& j)
	{
		IF_F(!j.is_object());

		m_name = j.value("name", "");
		m_class = j.value("class", "");
		m_bLog = j.value("bLog", false);

		IF_F(m_name.empty());
		IF_F(m_class.empty());

		return true;
	}

	bool BASE::link(const json& j, ModuleMgr* pM)
	{
		NULL_F(pM);

		return true;
	}


	int BASE::init(void *pKiss)
	{
		NULL__(pKiss, OK_ERR_NULLPTR);

		m_pKiss = pKiss;
		Kiss *pK = (Kiss *)pKiss;
		pK->setModule(this);

		pK->v("bLog", &m_bLog);

		return OK_OK;
	}

	int BASE::link(void)
	{
		NULL__(m_pKiss, OK_ERR_NULLPTR);

		return OK_OK;
	}

	int BASE::start(void)
	{
		return OK_OK;
	}

	int BASE::check(void)
	{
		return OK_OK;
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

	string BASE::getName(void)
	{
		if (!m_pKiss)
			return "";

		return ((Kiss *)m_pKiss)->getName();
	}

	string BASE::getClass(void)
	{
		if (!m_pKiss)
			return "";

		return ((Kiss *)m_pKiss)->getClass();
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

}
