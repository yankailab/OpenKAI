/*
 * _Console.cpp
 *
 *  Created on: Oct 13, 2019
 *      Author: Kai Yan
 */

#include "_Console.h"
#include "../Module/Kiss.h"

namespace kai
{

	_Console::_Console()
	{
		m_iY = 0;
	}

	_Console::~_Console()
	{
		endwin();
	}

	int _Console::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		initscr();
		noecho();
		cbreak();
		start_color();
		use_default_colors();
		init_pair(_Console_COL_TITLE, COLOR_WHITE, -1);
		init_pair(_Console_COL_NAME, COLOR_GREEN, -1);
		init_pair(_Console_COL_FPS, COLOR_YELLOW, -1);
		init_pair(_Console_COL_MSG, COLOR_WHITE, -1);
		init_pair(_Console_COL_ERROR, COLOR_RED, -1);

		return OK_OK;
	}

	int _Console::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		vector<string> vB;
		pK->a("vBASE", &vB);
		m_vpB.clear();
		for (string p : vB)
		{
			BASE *pB = (BASE *)(pK->findModule(p));
			IF_CONT(!pB);

			m_vpB.push_back(pB);
		}

		return OK_OK;
	}

	int _Console::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Console::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateConsole();

			m_pT->autoFPSto();
		}
	}

	void _Console::updateConsole(void)
	{
		move(0, 0);
		refresh();
		erase();

		attrset(COLOR_PAIR(_Console_COL_TITLE) | A_BOLD);
		mvaddstr(0, 1, this->getName().c_str());
		m_iY = 0;

		for (BASE *pB : m_vpB)
		{
			pB->console(this);
		}
	}

	void _Console::addMsg(const string &msg, int iLine)
	{
		addMsg(msg, COLOR_PAIR(_Console_COL_MSG), _Console_X_MSG, iLine);
	}

	void _Console::addMsg(const string &msg, int iCol, int iX, int iLine)
	{
		attrset(iCol);
		m_iY += iLine;
		mvaddstr(m_iY, iX, msg.c_str());
	}

}
