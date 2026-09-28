/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#include "BASE.h"
#include "../UI/_Console.h"

namespace kai
{

	BASE::BASE()
	{
	}

	BASE::~BASE()
	{
	}

	void BASE::setConfig(JsonCfg *pJcfg, json *pJ)
	{
		NULL_(pJcfg);
		NULL_(pJ);

		m_pJcfg = pJcfg;
		m_pJ = pJ;
	}

	void BASE::setName(const string &n)
	{
		m_name = n;
	}

	string BASE::getName(void)
	{
		return m_name;
	}

	string BASE::getClass(void)
	{
		return m_class;
	}

	bool BASE::loadConfig(void)
	{
		NULL_F(m_pJcfg);
		if (!m_pJ)
		{
			m_pJ = jK(*m_pJcfg->getJson(), m_name);
		}
		IF_F(!m_pJ || !m_pJ->is_object());

		const json &j = *m_pJ;
		jKv(j, "name", m_name);
		jKv(j, "class", m_class);
		jKv(j, "bLog", m_bLog);

		IF_Le_F(m_name.empty(), "name empty");
		IF_Le_F(m_class.empty(), "class empty");

		return true;
	}

	bool BASE::saveConfig(bool bExport)
	{
		IF_F(!m_pJcfg || !m_pJ || !m_pJ->is_object());

		// Update shared fields in place, preserving the rest of the document.
		(*m_pJ)["name"] = m_name;
		(*m_pJ)["class"] = m_class;
		(*m_pJ)["bLog"] = m_bLog;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	void BASE::console(void *pConsole)
	{
		NULL_(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("____________________________________", COLOR_PAIR(_Console_COL_NAME) | A_BOLD, _Console_X_NAME, 1);
		pC->addMsg(this->getName(), COLOR_PAIR(_Console_COL_NAME) | A_BOLD, _Console_X_NAME, 1);
	}

	void BASE::console(const json &j, void *pJSONbase)
	{
	}

}
