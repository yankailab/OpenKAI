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
        m_pGG = NULL;
        m_pGcell = NULL;
        m_nGcell = 0;
	}

	_SwarmSearch::~_SwarmSearch()
	{
        DEL(m_pGcell);
	}

	bool _SwarmSearch::init(void *pKiss)
	{
		IF_F(!this->_SwarmBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		int ie = USEC_1SEC;
		pK->v("ieOptRoute", &ie);
		m_ieOptRoute.init(ie);

		return true;
	}

	bool _SwarmSearch::link(void)
	{
		IF_F(!this->_SwarmBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("GeoGrid", &n);
		m_pGG = (GeoGrid *)(pK->getInst(n));
		IF_Fl(!m_pGG, n + ": not found");

		return true;
	}

    bool _SwarmSearch::start(void)
    {
        NULL_F(m_pT);
        IF_F(!m_pT->start(getUpdate, this));
    }

	int _SwarmSearch::check(void)
	{
        NULL__(m_pGG, -1);

		return this->_SwarmBase::check();
	}

    void _SwarmSearch::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPSfrom();

            if(!m_pGcell)
                genGridCells();

            updateNodes();

            optimizeRoute();

            m_pT->autoFPSto();
        }
    }

    void _SwarmSearch::updateNodes(void)
    {
        IF_(check() < 0);
    }

	bool _SwarmSearch::genGridCells(void)
    {
        IF_F(check() < 0);

        m_nGcell = m_pGG->getNcell();
        DEL(m_pGcell);
        m_pGcell = new GCELL_SEARCH[m_nGcell];
        NULL_F(m_pGcell);




    }

    void _SwarmSearch::optimizeRoute(void)
    {
        IF_(check() < 0);
    }

	void _SwarmSearch::handleMsgGCupdate(const SWMSG_GC_UPDATE& m)
    {

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
