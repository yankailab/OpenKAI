/*
 * _SwarmSearch.cpp
 *
 *  Created on: July 19, 2023
 *      Author: yankai
 */

#include "_SwarmSearch.h"

namespace kai
{

	_SwarmSearch::_SwarmSearch()
	{
	}

	_SwarmSearch::~_SwarmSearch()
	{
	}

	bool _SwarmSearch::init(void *pKiss)
	{
		IF_F(!this->_SwarmBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

//		pK->v("tExpire", &m_tExpire);

		return true;
	}

	bool _SwarmSearch::link(void)
	{
		IF_F(!this->_SwarmBase::link());

		return true;
	}

    bool _SwarmSearch::start(void)
    {
        NULL_F(m_pT);
        IF_F(!m_pT->start(getUpdate, this));
    }

	int _SwarmSearch::check(void)
	{
//        NULL__(m_pIO, -1);

		return this->_SwarmBase::check();
	}

    void _SwarmSearch::update(void)
    {
        while (m_pT->bRun())
        {
            // if (!m_pIO)
            // {
            //     m_pT->sleepT(SEC_2_USEC);
            //     continue;
            // }

            // if (!m_pIO->isOpen())
            // {
            //     if (!m_pIO->open())
            //     {
            //         m_pT->sleepT(SEC_2_USEC);
            //         continue;
            //     }
            // }

            m_pT->autoFPSfrom();

            updateNodes();

            m_pT->autoFPSto();
        }
    }

    void _SwarmSearch::updateNodes(void)
    {
        IF_(check() < 0);
    }

	void _SwarmSearch::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(check() < 0);
		this->_ModuleBase::console(pConsole);

		// string msg = "id=" + i2str(pO->getTopClass());
		// ((_Console *)pConsole)->addMsg(msg, 1);
	}

}
