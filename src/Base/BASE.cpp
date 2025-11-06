/*
 * BASE.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: root
 */

#include "BASE.h"
#include "../Module/Kiss.h"
#include "../UI/_Console.h"
#include "../Utility/utilFile.h"

namespace kai
{

	BASE::BASE()
	{
		m_pKiss = nullptr;
		m_bLog = false;

		m_fConfig = "";
		m_pKconfig = nullptr;
	}

	BASE::~BASE()
	{
	}

	int BASE::init(void *pKiss)
	{
		NULL__(pKiss, OK_ERR_NULLPTR);

		m_pKiss = pKiss;
		Kiss *pK = (Kiss *)pKiss;
		pK->setModule(this);

		pK->v("bLog", &m_bLog);

		pK->v("fConfig", &m_fConfig);
		IF__(m_fConfig.empty(), OK_OK);

		string s;
		IF__(!readFile(m_fConfig, &s), OK_ERR_NOT_FOUND);

		DEL(m_pKconfig);
		m_pKconfig = new Kiss();
		if (!((Kiss *)m_pKconfig)->parse(s))
		{
			DEL(m_pKconfig);
			return OK_ERR_INVALID_VALUE;
		}

		return OK_OK;
	}

	bool BASE::saveConfig(const string &fConfig, picojson::object* pO)
	{
		NULL_F(pO);

		string f = picojson::value(*pO).serialize();
		writeFile(fConfig, f);

		return true;
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

	int BASE::serialize(uint8_t *pB, int nB)
	{
		return OK_ERR_UNIMPLEMENTED;
	}

	int BASE::deSerialize(uint8_t *pB, int nB)
	{
		return OK_ERR_UNIMPLEMENTED;
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

	void BASE::context(void *pContext)
	{
	}

}
