#include "_PCgrid.h"

namespace kai
{

	_PCgrid::_PCgrid()
	{
		m_tExpire = 0;

		m_type = pc_grid;
		m_nCell = 0;
		m_nMedWidth = 3;
		m_vDim.clear();
		m_vPmin.clear();

		m_vPorigin.clear();
		m_vCellRangeX.clear();
		m_vCellRangeY.clear();
		m_vCellRangeZ.clear();
		m_vCellSize.clear();

		m_bVisual = false;
		m_gridLine.Clear();
		m_vAxisColX.set(1, 0, 0);
		m_vAxisColY.set(0, 1, 0);
		m_vAxisColZ.set(0, 0, 1);

		m_pCellActive = &m_pActiveCells[0];
	}

	_PCgrid::~_PCgrid()
	{
	}

	int _PCgrid::init(void *pKiss)
	{
		CHECK_(this->_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nMedWidth", &m_nMedWidth);

		pK->v("vPorigin", &m_vPorigin);
		pK->v("vCellRangeX", &m_vCellRangeX);
		pK->v("vCellRangeY", &m_vCellRangeY);
		pK->v("vCellRangeZ", &m_vCellRangeZ);
		pK->v("vCellSize", &m_vCellSize);

		pK->v("bVisual", &m_bVisual);
		pK->v("vAxisColX", &m_vAxisColX);
		pK->v("vAxisColY", &m_vAxisColY);
		pK->v("vAxisColZ", &m_vAxisColZ);

		pK->v("tExpire", &m_tExpire);

		return initGeometry();
	}

	int _PCgrid::link(void)
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

	int _PCgrid::initGeometry(void)
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
				m_pCell[i].m_nPactivate = 1;
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

	int _PCgrid::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _PCgrid::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _PCgrid::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			mutexLock();

			updatePCgrid();

			mutexUnlock();
		}
	}

	void _PCgrid::updatePCgrid(void)
	{
		IF_(check() != OK_OK);

		clearAllCells();

		for (_GeometryBase *pG : m_vpGB)
		{
			addGeometry(pG, m_tExpire);
		}

		updateActiveCell();
		updateActiveCellLS(m_pCellActive);

	}

	void _PCgrid::updateActiveCell(void)
	{
		//		mutexLock();

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

		//		mutexUnlock();
	}

	void _PCgrid::updateActiveCellLS(PC_GRID_ACTIVE_CELL *pAcell)
	{
		NULL_(pAcell);

		//		mutexLock();
		pAcell->clearLS();
		pAcell->generateLS(m_vRx, m_vRy, m_vRz, m_vCellSize);
		//		mutexUnlock();
	}

	void _PCgrid::getActiveCellLines(LineSet *pLS, int cIdx)
	{
		NULL_(pLS);
		IF_(cIdx >= PCGRID_ACTIVECELL_N);

		// TODO: lock needed?
		mutexLock();
		*pLS = m_pActiveCells[cIdx].m_LS;
		mutexUnlock();
	}

	void _PCgrid::addPCstream(void *p, const uint64_t &tExpire)
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

	void _PCgrid::addPCframe(void *p)
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

	void _PCgrid::setPorigin(const vFloat3 &vPo)
	{
		m_vPorigin = vPo;
	}

	void _PCgrid::setCellSize(const vFloat3 &vCsize)
	{
		m_vCellSize = vCsize;
	}

	void _PCgrid::setCellRangeX(const vInt2 &vX)
	{
		m_vCellRangeX = vX;
	}

	void _PCgrid::setCellRangeY(const vInt2 &vY)
	{
		m_vCellRangeY = vY;
	}

	void _PCgrid::setCellRangeZ(const vInt2 &vZ)
	{
		m_vCellRangeZ = vZ;
	}

	vFloat3 _PCgrid::getPorigin(void)
	{
		return m_vPorigin;
	}

	vFloat3 _PCgrid::getCellSize(void)
	{
		return m_vCellSize;
	}

	vInt2 _PCgrid::getCellRangeX(void)
	{
		return m_vCellRangeX;
	}

	vInt2 _PCgrid::getCellRangeY(void)
	{
		return m_vCellRangeY;
	}

	vInt2 _PCgrid::getCellRangeZ(void)
	{
		return m_vCellRangeZ;
	}

	void _PCgrid::clearAllCells(void)
	{
		//		mutexLock();

		for (int i = 0; i < m_nCell; i++)
			m_pCell[i].clear();

		//		mutexUnlock();
	}

	PC_GRID_CELL *_PCgrid::getCell(const vFloat3 &vP)
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

	PC_GRID_CELL *_PCgrid::getCell(const vInt3 &vC)
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

	int _PCgrid::getCell1Didx(const vInt3 &vC)
	{
		return vC.x * m_dYZ + vC.y * m_vDim.z + vC.z;
	}

	LineSet *_PCgrid::getGridLines(void)
	{
		return &m_gridLine;
	}

	void _PCgrid::generateGridLines(void)
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

	void _PCgrid::addGridAxisLine(int nDa,
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
