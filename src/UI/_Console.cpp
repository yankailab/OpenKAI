/*
 * _Console.cpp
 *
 *  Created on: Oct 13, 2019
 *      Author: Kai Yan
 */

#include "_Console.h"

namespace kai
{

	_Console::_Console()
	{
	}

	_Console::~_Console()
	{
		stop();
	}

	bool _Console::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());

		IF_F(initscr() == nullptr);
		m_bInitialized = true;
		noecho();
		cbreak();
		start_color();
		use_default_colors();
		init_pair(_Console_COL_TITLE, COLOR_WHITE, -1);
		init_pair(_Console_COL_NAME, COLOR_GREEN, -1);
		init_pair(_Console_COL_FPS, COLOR_YELLOW, -1);
		init_pair(_Console_COL_MSG, COLOR_WHITE, -1);
		init_pair(_Console_COL_ERROR, COLOR_RED, -1);

		return true;
	}

	bool _Console::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		const json &j = *m_pJ;

		vector<string> vB;
		jKv(j, "vBASE", vB);
		m_vpB.clear();
		for (string n : vB)
		{
			BASE *pB = (BASE *)(m_pM->findModule(n));
			IF_CONT(!pB);

			m_vpB.push_back(pB);
		}

		return true;
	}

	bool _Console::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _Console::stop(void)
	{
		// A final refresh after endwin() would put the terminal back into curses mode.
		if (m_pT)
			m_pT->join();

		if (m_bInitialized && !isendwin())
			endwin();
	}

	void _Console::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateConsole();
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
