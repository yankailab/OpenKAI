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

	void BASE::setModuleMgr(ModuleMgr *pM)
	{
		NULL_(pM);

		m_pM = pM;
	}

	void BASE::setConfig(JsonCfg *pJcfg, json *pJ)
	{
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
		// Parent layers establish the manager and mutable configuration first.
		if (!m_pM || m_name.empty())
		{
			return false;
		}

		if (!m_pJcfg)
		{
			m_pJcfg = m_pM->findJsonCfg(m_name);
		}
		if (!m_pJcfg)
		{
			return false;
		}
		if (!m_pJ)
		{
			m_pJ = jK(*m_pJcfg->getJson(), m_name);
		}
		if (!m_pJ || !m_pJ->is_object())
		{
			return false;
		}

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
		if (!m_pM || !m_pJcfg || !m_pJ || !m_pJ->is_object())
		{
			return false;
		}

		// Update shared fields in place, preserving the rest of the document.
		(*m_pJ)["name"] = m_name;
		(*m_pJ)["class"] = m_class;
		(*m_pJ)["bLog"] = m_bLog;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool BASE::link(void)
	{
		NULL_F(m_pM);
		NULL_F(m_pJcfg);
		NULL_F(m_pJ);

		/*
		 in each level of inheritance use m_pM to find the pointer to link modules
		 */

		return true;
	}

	bool BASE::start(void)
	{
		return true;
	}

	bool BASE::check(void)
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

	void BASE::draw(void *pMat)
	{
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
