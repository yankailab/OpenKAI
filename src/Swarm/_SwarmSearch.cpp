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
        m_pGcell = nullptr;
        m_nGcell = 0;
	}

	_SwarmSearch::~_SwarmSearch()
	{
        DEL(m_pGcell);
	}

	bool _SwarmSearch::init(const json& j)
	{
		IF_F(!this->_SwarmBase::init(j));

		int ie = USEC_1SEC;
		ie = j.value("ieOptRoute", "");
		m_ieOptRoute.init(ie);

		return true;
	}

	bool _SwarmSearch::link(const json& j, ModuleMgr* pM)
	{
		IF_F(!this->_SwarmBase::link(j, pM));

		return true;
	}

    bool _SwarmSearch::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

	bool _SwarmSearch::check(void)
	{
        NULL__(m_pGG);

		return this->_SwarmBase::check();
	}

    void _SwarmSearch::update(void)
    {
        while (m_pT->bAlive())
        {
            m_pT->autoFPS();

            if(!m_pGcell)
                genGridCells();

            updateNodes();

            optimizeRoute();


        }
    }

    void _SwarmSearch::updateNodes(void)
    {
        IF_(!check());
    }

	bool _SwarmSearch::genGridCells(void)
    {
        IF_F(!check());

        m_nGcell = m_pGG->getNcell();
		IF_F(m_nGcell <= 0);

        DEL(m_pGcell);
        m_pGcell = new GCELL_SEARCH[m_nGcell];
        NULL_F(m_pGcell);

		clearAllGridCells();
    }

    void _SwarmSearch::optimizeRoute(void)
    {
        IF_(!check());
    }

	void _SwarmSearch::handleMsgGCupdate(const SWMSG_GC_UPDATE& m)
    {
		NULL_(m_pGcell);
		if(m.m_iGC == 0xFFFFFFFF)
		{
			clearAllGridCells();
			return;
		}
		IF_(m.m_iGC >= m_nGcell);

		m_pGcell[m.m_iGC].m_w += m.m_w;
    }

	void _SwarmSearch::clearAllGridCells(void)
	{
		NULL_(m_pGcell);

		for(int i =0; i<m_nGcell; i++)
			m_pGcell[i].clear();
	}

	void _SwarmSearch::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(!check());
		this->_SwarmBase::console(pConsole);

		// string msg = "id=" + i2str(pO->getTopClass());
		// ((_Console *)pConsole)->addMsg(msg, 1);
	}

}
