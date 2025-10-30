#include "_PCgridBase.h"

namespace kai
{

	_PCgridBase::_PCgridBase()
	{
		m_type = pc_grid;
		m_nCell = 0;
		m_vDim.clear();
		m_vPmin.clear();

		m_fGridConfig = "";

		m_vPorigin.clear();
		m_vCellRangeX.clear();
		m_vCellRangeY.clear();
		m_vCellRangeZ.clear();
		m_vCellSize.clear();

		m_bVisual = false;
		m_vAxisColX.set(1, 0, 0);
		m_vAxisColY.set(0, 1, 0);
		m_vAxisColZ.set(0, 0, 1);

		m_nMedWidth = 3;
		m_tExpire = 0;

		m_gridLine.Clear();
		m_pCellActive = &m_pActiveCells[0];
	}

	_PCgridBase::~_PCgridBase()
	{
	}

	int _PCgridBase::init(void *pKiss)
	{
		CHECK_(this->_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vPorigin", &m_vPorigin);
		pK->v("vCellRangeX", &m_vCellRangeX);
		pK->v("vCellRangeY", &m_vCellRangeY);
		pK->v("vCellRangeZ", &m_vCellRangeZ);
		pK->v("vCellSize", &m_vCellSize);

		pK->v("bVisual", &m_bVisual);
		pK->v("vAxisColX", &m_vAxisColX);
		pK->v("vAxisColY", &m_vAxisColY);
		pK->v("vAxisColZ", &m_vAxisColZ);

		pK->v("nMedWidth", &m_nMedWidth);
		pK->v("tExpire", &m_tExpire);

		pK->v("fGridConfig", &m_fGridConfig);

		return OK_OK;
	}

	int _PCgridBase::link(void)
	{
		CHECK_(this->_GeometryBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		vector<string> vGn;
		pK->a("vGeometryBase", &vGn);
		for (string n : vGn)
		{
			_GeometryBase *pG = (_GeometryBase *)(pK->findModule(n));
			IF_CONT(!pG);

			m_vpGB.push_back(pG);
		}

		return OK_OK;
	}

	string _PCgridBase::getConfigFileName(void)
	{
		return m_fGridConfig;
	}

	bool _PCgridBase::loadConfig(const string &fName, Kiss* pK)
	{
		NULL_F(pK);

		string s;
		IF_F(!readFile(fName, &s));

		if (!pK->parse(s))
		{
			LOG_I("Config load failed: " + fName);
			return false;
		}

		Kiss *pKc = pK->root()->child("gridConfig");
		IF_F(pKc->empty());

		// grid config
		pKc->v("vPorigin", &m_vPorigin);
		pKc->v("vCellRangeX", &m_vCellRangeX);
		pKc->v("vCellRangeY", &m_vCellRangeY);
		pKc->v("vCellRangeZ", &m_vCellRangeZ);
		pKc->v("vCellSize", &m_vCellSize);

		pKc->v("bVisual", &m_bVisual);
		pKc->v("vAxisColX", &m_vAxisColX);
		pKc->v("vAxisColY", &m_vAxisColY);
		pKc->v("vAxisColZ", &m_vAxisColZ);

		pKc->v("nMedWidth", &m_nMedWidth);
		pKc->v("tExpire", &m_tExpire);

		return true;
	}

	bool _PCgridBase::saveConfig(const string &fName, picojson::object* pO)
	{
		NULL_F(pO);

		pO->insert(make_pair("name", "gridConfig"));

		// grid config
		picojson::array vA;
		vA.push_back(value((double)m_vPorigin.x));
		vA.push_back(value((double)m_vPorigin.y));
		vA.push_back(value((double)m_vPorigin.z));
		pO->insert(make_pair("vPorigin", vA));

		vA.clear();
		vA.push_back(value((double)m_vCellRangeX.x));
		vA.push_back(value((double)m_vCellRangeX.y));
		pO->insert(make_pair("vCellRangeX", vA));

		vA.clear();
		vA.push_back(value((double)m_vCellRangeY.x));
		vA.push_back(value((double)m_vCellRangeY.y));
		pO->insert(make_pair("vCellRangeY", vA));

		vA.clear();
		vA.push_back(value((double)m_vCellRangeZ.x));
		vA.push_back(value((double)m_vCellRangeZ.y));
		pO->insert(make_pair("vCellRangeZ", vA));

		vA.clear();
		vA.push_back(value((double)m_vCellSize.x));
		vA.push_back(value((double)m_vCellSize.y));
		vA.push_back(value((double)m_vCellSize.z));
		pO->insert(make_pair("vCellSize", vA));

		pO->insert(make_pair("bVisual", value((double)m_bVisual)));

		vA.clear();
		vA.push_back(value((double)m_vAxisColX.x));
		vA.push_back(value((double)m_vAxisColX.y));
		vA.push_back(value((double)m_vAxisColX.z));
		pO->insert(make_pair("vAxisColX", vA));

		vA.clear();
		vA.push_back(value((double)m_vAxisColY.x));
		vA.push_back(value((double)m_vAxisColY.y));
		vA.push_back(value((double)m_vAxisColY.z));
		pO->insert(make_pair("vAxisColY", vA));

		vA.clear();
		vA.push_back(value((double)m_vAxisColZ.x));
		vA.push_back(value((double)m_vAxisColZ.y));
		vA.push_back(value((double)m_vAxisColZ.z));
		pO->insert(make_pair("vAxisColZ", vA));

		pO->insert(make_pair("nMedWidth", value((double)m_nMedWidth)));
		pO->insert(make_pair("tExpire", value((double)m_tExpire)));

		string f = picojson::value(*pO).serialize();
		writeFile(fName, f);

		return true;
	}

	int _PCgridBase::initGeometry(void)
	{
		float cVol = m_vCellSize.x * m_vCellSize.y * m_vCellSize.z;
		IF__(cVol <= 0, OK_ERR_INVALID_VALUE);

		// calc grid size
		vInt3 vDim;
		vDim.set(m_vCellRangeX.len(),
				 m_vCellRangeY.len(),
				 m_vCellRangeZ.len());
		int nCell = vDim.x * vDim.y * vDim.z;
		IF__(nCell <= 0, OK_ERR_INVALID_VALUE);
		IF__(nCell > PC_GRID_N_CELL, OK_ERR_INVALID_VALUE);

		m_vDim = vDim;

		mutexLock();

		// alllocate cells
		if (m_pCell)
		{
			m_nCell = nCell;

			for (int i = 0; i < m_nCell; i++)
			{
				m_pCell[i].init(m_nMedWidth);
				m_pCell[i].m_nPactivate = 0;
			}

			// prepare quick access params
			m_vRx.set(m_vPorigin.x + m_vCellRangeX.x * m_vCellSize.x,
					  m_vPorigin.x + m_vCellRangeX.y * m_vCellSize.x);

			m_vRy.set(m_vPorigin.y + m_vCellRangeY.x * m_vCellSize.y,
					  m_vPorigin.y + m_vCellRangeY.y * m_vCellSize.y);

			m_vRz.set(m_vPorigin.z + m_vCellRangeZ.x * m_vCellSize.z,
					  m_vPorigin.z + m_vCellRangeZ.y * m_vCellSize.z);

			m_vPmin.set(m_vRx.x, m_vRy.x, m_vRz.x);

			m_vOvCellSize.set(1.0 / m_vCellSize.x,
							  1.0 / m_vCellSize.y,
							  1.0 / m_vCellSize.z);

			m_dYZ = m_vDim.y * m_vDim.z;

			// generate line set for grid visualization
			if (m_bVisual)
			{
				generateGridLines();
			}
		}

		mutexUnlock();

		return OK_OK;
	}

	int _PCgridBase::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _PCgridBase::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _PCgridBase::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			mutexLock();

			updatePCgrid();

			mutexUnlock();
		}
	}

	void _PCgridBase::updatePCgrid(void)
	{
		IF_(check() != OK_OK);

		clearAllCells();

		for (_GeometryBase *pG : m_vpGB)
		{
			addGeometry(pG, m_tExpire);
		}

		updateActiveCell();
		updateActiveCellLS(m_pCellActive, true);

	}

	void _PCgridBase::updateActiveCell(void)
	{
		m_pCellActive->clearCells();

		vInt3 vC;
		for (int i = 0; i < m_vDim.x; i++)
		{
			for (int j = 0; j < m_vDim.y; j++)
			{
				for (int k = 0; k < m_vDim.z; k++)
				{
					vC.set(i, j, k);
					PC_GRID_CELL *pC = getCell(vC);
					IF_CONT(!pC);
					pC->updateFilter();

					IF_CONT(pC->nP() < 1);

					m_pCellActive->addCell(vC);
				}
			}
		}
	}

	void _PCgridBase::updateActiveCellLS(PC_GRID_ACTIVE_CELL *pAcell, bool bBBox)
	{
		NULL_(pAcell);

		pAcell->clearLS();
		pAcell->generateLS(m_vRx, m_vRy, m_vRz, m_vCellSize);

		if(bBBox)
			pAcell->generateBBoxLS(m_vRx, m_vRy, m_vRz);
	}

	void _PCgridBase::getActiveCellLines(LineSet *pLS, int cIdx)
	{
		NULL_(pLS);
		IF_(cIdx >= PCGRID_ACTIVECELL_N);

		// TODO: lock needed?
		mutexLock();
		*pLS = m_pActiveCells[cIdx].m_LS;
		mutexUnlock();
	}

	void _PCgridBase::addPCstream(void *p, const uint64_t &tExpire)
	{
		IF_(check() != OK_OK);
		NULL_(p);
		_PCstream *pS = (_PCstream *)p;

		uint64_t tNow = getApproxTbootUs();
		int nP = pS->nP();

		for (int i = 0; i < nP; i++)
		{
			GEOMETRY_POINT *pP = pS->get(i);
			if (tExpire)
			{
				IF_CONT(bExpired(pP->m_tStamp, tExpire, tNow));
			}

			PC_GRID_CELL *pC = getCell(vFloat3(pP->m_vP[0], pP->m_vP[1], pP->m_vP[2]));
			IF_CONT(!pC);

			pC->add(1);
		}
	}

	void _PCgridBase::addPCframe(void *p)
	{
		IF_(check() != OK_OK);
		NULL_(p);
		_PCframe *pF = (_PCframe *)p;

		PointCloud *pPC = pF->getBuffer();
		for (int i = 0; i < pPC->points_.size(); i++)
		{
			Vector3f vP = pPC->points_[i].cast<float>();
			PC_GRID_CELL *pC = getCell(vFloat3(vP[0], vP[1], vP[2]));
			IF_CONT(!pC);

			pC->add(1);
		}
	}

	void _PCgridBase::setPorigin(const vFloat3 &vPo)
	{
		m_vPorigin = vPo;
	}

	void _PCgridBase::setCellSize(const vFloat3 &vCsize)
	{
		m_vCellSize = vCsize;
	}

	void _PCgridBase::setCellRangeX(const vInt2 &vX)
	{
		m_vCellRangeX = vX;
	}

	void _PCgridBase::setCellRangeY(const vInt2 &vY)
	{
		m_vCellRangeY = vY;
	}

	void _PCgridBase::setCellRangeZ(const vInt2 &vZ)
	{
		m_vCellRangeZ = vZ;
	}

	vFloat3 _PCgridBase::getPorigin(void)
	{
		return m_vPorigin;
	}

	vFloat3 _PCgridBase::getCellSize(void)
	{
		return m_vCellSize;
	}

	vInt2 _PCgridBase::getCellRangeX(void)
	{
		return m_vCellRangeX;
	}

	vInt2 _PCgridBase::getCellRangeY(void)
	{
		return m_vCellRangeY;
	}

	vInt2 _PCgridBase::getCellRangeZ(void)
	{
		return m_vCellRangeZ;
	}

	void _PCgridBase::clearAllCells(void)
	{
		//		mutexLock();

		for (int i = 0; i < m_nCell; i++)
			m_pCell[i].clear();

		//		mutexUnlock();
	}

	PC_GRID_CELL *_PCgridBase::getCell(const vFloat3 &vP)
	{
		IF__(isnan(vP.x), nullptr);
		IF__(isnan(vP.y), nullptr);
		IF__(isnan(vP.z), nullptr);

		IF__(!m_vRx.bInside(vP.x), nullptr);
		IF__(!m_vRy.bInside(vP.y), nullptr);
		IF__(!m_vRz.bInside(vP.z), nullptr);

		vInt3 vPi;
		vPi.x = (vP.x - m_vPmin.x) * m_vOvCellSize.x;
		vPi.y = (vP.y - m_vPmin.y) * m_vOvCellSize.y;
		vPi.z = (vP.z - m_vPmin.z) * m_vOvCellSize.z;

		return getCell(vPi);
	}

	PC_GRID_CELL *_PCgridBase::getCell(const vInt3 &vC)
	{
		IF__(vC.x < 0, nullptr);
		IF__(vC.x >= m_vDim.x, nullptr);
		IF__(vC.y < 0, nullptr);
		IF__(vC.y >= m_vDim.y, nullptr);
		IF__(vC.z < 0, nullptr);
		IF__(vC.z >= m_vDim.z, nullptr);

		int i = vC.x * m_dYZ + vC.y * m_vDim.z + vC.z;
		return &m_pCell[i];
	}

	int _PCgridBase::getCell1Didx(const vInt3 &vC)
	{
		return vC.x * m_dYZ + vC.y * m_vDim.z + vC.z;
	}

	LineSet *_PCgridBase::getGridLines(void)
	{
		return &m_gridLine;
	}

	void _PCgridBase::generateGridLines(void)
	{
		m_gridLine.Clear();

		addGridAxisLine(m_vDim.x,
						m_vRx,
						m_vCellSize.x,
						m_vDim.y,
						m_vRy,
						m_vCellSize.y,
						m_vRz,
						vInt3(0, 1, 2),
						vFloat3(1, 0, 0));

		addGridAxisLine(m_vDim.x,
						m_vRx,
						m_vCellSize.x,
						m_vDim.z,
						m_vRz,
						m_vCellSize.z,
						m_vRy,
						vInt3(0, 2, 1),
						vFloat3(0, 1, 0));

		addGridAxisLine(m_vDim.y,
						m_vRy,
						m_vCellSize.y,
						m_vDim.z,
						m_vRz,
						m_vCellSize.z,
						m_vRx,
						vInt3(1, 2, 0),
						vFloat3(0, 0, 1));
	}

	void _PCgridBase::addGridAxisLine(int nDa,
								  const vFloat2 &vRa,
								  float csA,
								  int nDb,
								  const vFloat2 &vRb,
								  float csB,
								  const vFloat2 &vRc,
								  const vInt3 &vAxis,
								  const vFloat3 &vCol)
	{
		int iA, iB;
		Vector3f pA, pB;
		float *pAx = &pA[vAxis.x];
		float *pAy = &pA[vAxis.y];
		float *pAz = &pA[vAxis.z];
		float *pBx = &pB[vAxis.x];
		float *pBy = &pB[vAxis.y];
		float *pBz = &pB[vAxis.z];

		for (iA = 0; iA < nDa; iA++)
		{
			*pAx = vRa.x + csA * iA;
			*pAz = vRc.x;

			for (iB = 0; iB < nDb; iB++)
			{
				*pAy = vRb.x + csB * iB;

				*pBx = *pAx;
				*pBy = *pAy;
				*pBz = vRc.y;

				int nP = m_gridLine.points_.size();
				m_gridLine.points_.push_back(pA.cast<double>());
				m_gridLine.points_.push_back(pB.cast<double>());
				m_gridLine.lines_.push_back(Vector2i{nP, nP + 1});
				m_gridLine.colors_.push_back(Vector3d{vCol.x, vCol.y, vCol.z});
			}
		}
	}

}
