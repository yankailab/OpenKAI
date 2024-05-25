#include "_GSVgrid.h"

namespace kai
{

	_GSVgrid::_GSVgrid()
	{
		m_pGio = NULL;
		m_bAlarm = false;

		m_pCellAlert = &m_pActiveCells[1];
		m_pCellAlarm = &m_pActiveCells[2];
		m_pCellSelected = &m_pActiveCells[3];
	}

	_GSVgrid::~_GSVgrid()
	{
	}

	bool _GSVgrid::init(void *pKiss)
	{
		IF_F(!this->_PCgrid::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;


		// test
		GSV_GRID_CELL gc;
		gc.m_vCellIdx = {10,10,10};
		gc.m_nPalarm = 29000;

		m_pCellAlert->clearCellIdx();
		m_pCellAlert->addCell(gc.m_vCellIdx);
		m_pCellAlert->clearLS();
		m_pCellAlert->generateLS(m_vRx, m_vRy, m_vRz, m_vCellSize);

		PC_GRID_CELL* pC = getCell(gc.m_vCellIdx);
		pC->m_nPactivate = gc.m_nPalarm;


		return true;
	}

	bool _GSVgrid::link(void)
	{
		IF_F(!this->_PCgrid::link());
		Kiss *pK = (Kiss *)m_pKiss;

        string n;
        n = "";
        pK->v("_GSVio", &n);
        m_pGio = (_GSVio *)(pK->getInst(n));

		return true;
	}

    bool _GSVgrid::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

	int _GSVgrid::check(void)
	{
		NULL__(m_pGio, -1);

		return this->_PCgrid::check();
	}

	void _GSVgrid::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updatePCgrid();

			updateGSVgrid();

			m_pT->autoFPSto();
		}
	}

	void _GSVgrid::updateGSVgrid(void)
	{
		IF_(check() < 0);

		// alarm cell
		m_pCellAlarm->clearCellIdx();

		for(vInt3 vCi : m_pCellAlert->m_vCellIdx)
		{
			PC_GRID_CELL* pC = getCell(vCi);
			IF_CONT(!pC);
			IF_CONT(pC->m_nP < pC->m_nPactivate);

			m_pCellAlarm->addCell(vCi);
		}

		m_bAlarm = !m_pCellAlarm->empty();
		m_pGio->setAlarm(m_bAlarm);

		mutexLock();
		m_pCellAlarm->clearLS();
		m_pCellAlarm->generateLS(m_vRx, m_vRy, m_vRz, m_vCellSize);
		mutexUnlock();

		// alert cell
		mutexLock();
		m_pCellAlert->clearLS();
		m_pCellAlert->generateLS(m_vRx, m_vRy, m_vRz, m_vCellSize);
		mutexUnlock();
		



	}


}
