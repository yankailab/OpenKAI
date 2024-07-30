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

	int _GSVgrid::init(void *pKiss)
	{
		CHECK_(this->_PCgrid::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nPalertSensitivity", &m_nPalertSensitivity);
		pK->v("vCselected", &m_vCselected);
		pK->v("fConfig", &m_fConfig);

		loadConfig();

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

		return OK_OK;
	}

	int _GSVgrid::link(void)
	{
		CHECK_(this->_PCgrid::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_GSVio", &n);
		m_pGio = (_GSVio *)(pK->findModule(n));
		NULL__(m_pGio, OK_ERR_NULLPTR);

		return OK_OK;
	}

	bool _GSVgrid::loadConfig(void)
	{
		string s;
		IF_F(!readFile(m_fConfig, &s));

		Kiss *pK = new Kiss();
		if(!pK->parse(s))
		{
			LOG_I("Config load failed: " + m_fConfig);
			DEL(pK);
			return false;
		}

		// TODO: organize the json obj
		Kiss *pKc = pK->root()->child("cellAlert");
		if (pKc->empty())
		{
			DEL(pK);
			return false;
		}

		// grid config
		pKc->v("vPoX", &m_vPorigin.x);
		pKc->v("vPoY", &m_vPorigin.y);
		pKc->v("vPoZ", &m_vPorigin.z);

		pKc->v("vGridXx", &m_vX.x);
		pKc->v("vGridXy", &m_vX.y);

		pKc->v("vGridYx", &m_vY.x);
		pKc->v("vGridYy", &m_vY.y);

		pKc->v("vGridZx", &m_vZ.x);
		pKc->v("vGridZy", &m_vZ.y);

		pKc->v("vCsizeX", &m_vCellSize.x);
		pKc->v("vCsizeY", &m_vCellSize.y);
		pKc->v("vCsizeZ", &m_vCellSize.z);

		initGrid();

		// alert cells
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
		calibAlertCellNpAlarm();

		picojson::object o;

		// TODO: organize the json obj
		o.insert(make_pair("name", "cellAlert"));

		// grid config
		o.insert(make_pair("vPoX", m_vPorigin.x));
		o.insert(make_pair("vPoY", m_vPorigin.y));
		o.insert(make_pair("vPoZ", m_vPorigin.z));

		o.insert(make_pair("vGridXx", (double)m_vX.x));
		o.insert(make_pair("vGridXy", (double)m_vX.y));

		o.insert(make_pair("vGridYx", (double)m_vY.x));
		o.insert(make_pair("vGridYy", (double)m_vY.y));

		o.insert(make_pair("vGridZx", (double)m_vZ.x));
		o.insert(make_pair("vGridZy", (double)m_vZ.y));

		o.insert(make_pair("vCsizeX", m_vCellSize.x));
		o.insert(make_pair("vCsizeY", m_vCellSize.y));
		o.insert(make_pair("vCsizeZ", m_vCellSize.z));

		// alert cells
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

		writeFile(m_fConfig, f);

		return true;
	}

	int _GSVgrid::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _GSVgrid::check(void)
	{
		NULL__(m_pGio, OK_ERR_NULLPTR);

		return this->_PCgrid::check();
	}

	void _GSVgrid::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			mutexLock();

			updatePCgrid();

			updateGSVgrid();

			mutexUnlock();

			m_pT->autoFPSto();
		}
	}

	void _GSVgrid::updateGSVgrid(void)
	{
		IF_(check() != OK_OK);

		// alarm cell
		m_pCellAlarm->clearCells();

		for (vInt3 vCi : m_pCellAlert->m_vCellIdx)
		{
			PC_GRID_CELL *pC = getCell(vCi);
			IF_CONT(!pC);
			IF_CONT(pC->nP() < pC->m_nPactivate + m_nPalertSensitivity);

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

	bool _GSVgrid::selectCell(const vInt3 vC)
	{
		PC_GRID_CELL *pC = getCell(vC);
		NULL_F(pC);

		m_vCselected = vC;

		m_pCellSelected->clearCells();
		m_pCellSelected->addCell(m_vCselected);
		//		updateActiveCellLS(m_pCellSelected);
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

			pC->m_nPactivate = pC->nP();
		}
	}

	void _GSVgrid::setNpAlertSensitivity(int s)
	{
		m_nPalertSensitivity = s;
	}

	void _GSVgrid::autoFindAlertCells(void)
	{
		m_pCellAlert->clearCells();

		int iX;
		int iY;
		int iZ;

		iX = 0;
		for (iY = 0; iY < m_vDim.y; iY++)
		{
			for (iZ = 0; iZ < m_vDim.z; iZ++)
			{
				m_pCellAlert->addCell(vInt3(iX, iY, iZ));
			}
		}

		iX = m_vDim.x-1;
		for (iY = 0; iY < m_vDim.y; iY++)
		{
			for (iZ = 0; iZ < m_vDim.z; iZ++)
			{
				m_pCellAlert->addCell(vInt3(iX, iY, iZ));
			}
		}


		iY = 0;
		for (iX = 0; iX < m_vDim.x; iX++)
		{
			for (iZ = 0; iZ < m_vDim.z; iZ++)
			{
				m_pCellAlert->addCell(vInt3(iX, iY, iZ));
			}
		}

		iY = m_vDim.y-1;
		for (iX = 0; iX < m_vDim.x; iX++)
		{
			for (iZ = 0; iZ < m_vDim.z; iZ++)
			{
				m_pCellAlert->addCell(vInt3(iX, iY, iZ));
			}
		}

	}

}
