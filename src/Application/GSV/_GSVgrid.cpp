#include "_GSVgrid.h"

namespace kai
{

	_GSVgrid::_GSVgrid()
	{
		m_fConfig = "";
		m_pGio = NULL;
		m_bAlarm = false;
		m_nPalertSensitivity = 10;
		m_vCselected.set(0, 0, 0);

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

		pK->v("nPalertSensitivity", &m_nPalertSensitivity);
		pK->v("vCselected", &m_vCselected);
		pK->v("fConfig", &m_fConfig);

		if (!loadConfig())
		{
			LOG_I("Config load failed");
		}

		m_pCellSelected->clearCells();
		m_pCellSelected->addCell(m_vCselected);


		// test
		// vInt3 vCellIdx = {5, 5, 5};
		// int nPalarm = 10000;

		// m_pCellAlert->clearCells();
		// m_pCellAlert->addCell(vCellIdx);

		// updateActiveCellLS(m_pCellAlert);

		// PC_GRID_CELL *pC = getCell(vCellIdx);
		// pC->m_nPactivate = nPalarm;

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
        IF_Fl(!m_pGio, n + ": not found");

		return true;
	}

	bool _GSVgrid::loadConfig(void)
	{
		Kiss *pK = new Kiss();

		if (!parseKiss(m_fConfig, pK))
		{
			DEL(pK);
			return false;
		}

		Kiss *pKc = pK->root()->child("cellAlert");
		if (pKc->empty())
		{
			DEL(pK);
			return false;
		}

		vector<int> vX, vY, vZ, vNpAlarm;
		pKc->a("vX", &vX);
		pKc->a("vY", &vY);
		pKc->a("vZ", &vZ);
		pKc->a("vNpAlarm", &vNpAlarm);
		DEL(pKc);

		m_pCellAlert->clearCells();
		for (int i = 0; i < vX.size(); i++)
		{
			vInt3 vC(vX[i], vY[i], vZ[i]);
			PC_GRID_CELL *pC = getCell(vC);
			IF_CONT(!pC);

			m_pCellAlert->addCell(vC);
			pC->m_nPactivate = vNpAlarm[i];
		}

		updateActiveCellLS(m_pCellAlert);

		return true;
	}

	bool _GSVgrid::saveConfig(void)
	{
//		calibAlertCellNpAlarm();

		picojson::object o;
		o.insert(make_pair("name", "cellAlert"));

		picojson::array vX, vY, vZ, vNpAlarm;
		for (vInt3 vC : m_pCellAlert->m_vCellIdx)
		{
			PC_GRID_CELL *pC = getCell(vC);
			IF_CONT(!pC);

			vX.push_back(value((double)vC.x));
			vY.push_back(value((double)vC.y));
			vZ.push_back(value((double)vC.z));
			vNpAlarm.push_back(value((double)pC->m_nPactivate));
		}

		o.insert(make_pair("vX", vX));
		o.insert(make_pair("vY", vY));
		o.insert(make_pair("vZ", vZ));
		o.insert(make_pair("vNpAlarm", vNpAlarm));

		string f = picojson::value(o).serialize();

		_File *pF = new _File();
		IF_F(!pF->open(m_fConfig, ios::out));
		pF->write((uint8_t *)f.c_str(), f.length());
		pF->close();
		DEL(pF);

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
		m_pCellAlarm->clearCells();

		for (vInt3 vCi : m_pCellAlert->m_vCellIdx)
		{
			PC_GRID_CELL *pC = getCell(vCi);
			IF_CONT(!pC);
			IF_CONT(pC->m_nP < pC->m_nPactivate);

			m_pCellAlarm->addCell(vCi);
		}

		m_bAlarm = !m_pCellAlarm->empty();
		m_pGio->setAlarm(m_bAlarm);

		updateActiveCellLS(m_pCellAlarm);

		// alert cell
		updateActiveCellLS(m_pCellAlert);

		// selected cell
		updateActiveCellLS(m_pCellSelected);
	}

	bool _GSVgrid::addAlertCell(const vInt3 vC)
	{
		return m_pCellAlert->addCell(vC);
	}

	void _GSVgrid::deleteAlertCell(const vInt3 vC)
	{
		return m_pCellAlert->deleteCell(vC);
	}

	void _GSVgrid::calibAlertCellNpAlarm(void)
	{
		for (vInt3 vC : m_pCellAlert->m_vCellIdx)
		{
			PC_GRID_CELL *pC = getCell(vC);
			IF_CONT(!pC);

			pC->m_nPactivate = pC->m_nP;
		}
	}

	bool _GSVgrid::selectCell(const vInt3 vC)
	{
		PC_GRID_CELL *pC = getCell(vC);
		NULL_F(pC);

		m_vCselected = vC;

		m_pCellSelected->clearCells();
		m_pCellSelected->addCell(m_vCselected);
//		updateActiveCellLS(m_pCellSelected);
	}

}
