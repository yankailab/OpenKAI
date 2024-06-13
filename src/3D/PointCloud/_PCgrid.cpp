#include "_PCgrid.h"

namespace kai
{

	_PCgrid::_PCgrid()
	{
		m_tExpire = 0;

		m_type = pc_grid;
//		m_pCell = NULL;
		m_nCell = 0;
		m_nMedWidth = 3;
		m_vDim.clear();
		m_vPmin.clear();

		m_vPorigin.clear();
		m_vX.clear();
		m_vY.clear();
		m_vZ.clear();
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
//		DEL(m_pCell);
	}

	bool _PCgrid::init(void *pKiss)
	{
		IF_F(!this->_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nMedWidth", &m_nMedWidth);

		pK->v("vPorigin", &m_vPorigin);
		pK->v("vX", &m_vX);
		pK->v("vY", &m_vY);
		pK->v("vZ", &m_vZ);
		pK->v("vCellSize", &m_vCellSize);

		pK->v("bVisual", &m_bVisual);
		pK->v("vAxisColX", &m_vAxisColX);
		pK->v("vAxisColY", &m_vAxisColY);
		pK->v("vAxisColZ", &m_vAxisColZ);

		pK->v("tExpire", &m_tExpire);

		return initGrid();
	}

	bool _PCgrid::link(void)
	{
		IF_F(!this->_GeometryBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		vector<string> vGn;
		pK->a("vGeometryBase", &vGn);
		for (string n : vGn)
		{
			_GeometryBase *pG = (_GeometryBase *)(pK->findModule(n));
			IF_CONT(!pG);

			m_vpGB.push_back(pG);
		}

		return true;
	}

	bool _PCgrid::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _PCgrid::setPorigin(const vFloat3 &vPo)
	{
		m_vPorigin = vPo;
	}

	void _PCgrid::setCellSize(const vFloat3 &vCsize)
	{
		m_vCellSize = vCsize;
	}

	void _PCgrid::setGridX(const vInt2 &vX)
	{
		m_vX = vX;
	}

	void _PCgrid::setGridY(const vInt2 &vY)
	{
		m_vY = vY;
	}

	void _PCgrid::setGridZ(const vInt2 &vZ)
	{
		m_vZ = vZ;
	}

	vFloat3 _PCgrid::getPorigin(void)
	{
		return m_vPorigin;
	}

	vFloat3 _PCgrid::getCellSize(void)
	{
		return m_vCellSize;
	}

	vInt2 _PCgrid::getGridX(void)
	{
		return m_vX;
	}

	vInt2 _PCgrid::getGridY(void)
	{
		return m_vY;
	}

	vInt2 _PCgrid::getGridZ(void)
	{
		return m_vZ;
	}

	bool _PCgrid::initGrid(void)
	{
		float cVol = m_vCellSize.x * m_vCellSize.y * m_vCellSize.z;
		IF_F(cVol <= 0);

		// calc grid size
		vInt3 vDim;
		vDim.set(m_vX.len(),
				 m_vY.len(),
				 m_vZ.len());
		int nCell = vDim.x * vDim.y * vDim.z;
		IF_F(nCell <= 0);

		m_vDim = vDim;

		mutexLock();

		// alllocate cells
//		DEL(m_pCell);

//		m_pCell = new PC_GRID_CELL[nCell];
		if (m_pCell)
		{
			m_nCell = nCell;

			for (int i = 0; i < m_nCell; i++)
			{
				m_pCell[i].init(m_nMedWidth);
				m_pCell[i].m_nPactivate = 1;
			}

			// prepare quick access params
			m_vRx.set(m_vPorigin.x + m_vX.x * m_vCellSize.x,
					  m_vPorigin.x + m_vX.y * m_vCellSize.x);

			m_vRy.set(m_vPorigin.y + m_vY.x * m_vCellSize.y,
					  m_vPorigin.y + m_vY.y * m_vCellSize.y);

			m_vRz.set(m_vPorigin.z + m_vZ.x * m_vCellSize.z,
					  m_vPorigin.z + m_vZ.y * m_vCellSize.z);

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

		return true;
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
		IF_N(isnan(vP.x));
		IF_N(isnan(vP.y));
		IF_N(isnan(vP.z));

		IF_N(!m_vRx.bInside(vP.x));
		IF_N(!m_vRy.bInside(vP.y));
		IF_N(!m_vRz.bInside(vP.z));

		vInt3 vPi;
		vPi.x = (vP.x - m_vPmin.x) * m_vOvCellSize.x;
		vPi.y = (vP.y - m_vPmin.y) * m_vOvCellSize.y;
		vPi.z = (vP.z - m_vPmin.z) * m_vOvCellSize.z;

		return getCell(vPi);
	}

	int _PCgrid::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _PCgrid::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			mutexLock();

			updatePCgrid();

			mutexUnlock();

			m_pT->autoFPSto();
		}
	}

	void _PCgrid::updatePCgrid(void)
	{
		IF_(check() < 0);

		clearAllCells();

		for (_GeometryBase *pG : m_vpGB)
		{
			addGeometry(pG, m_tExpire);
		}

		updateActiveCell();
		updateActiveCellLS(m_pCellActive);
	}

	void _PCgrid::addPCstream(void *p, const uint64_t &tExpire)
	{
		IF_(check() < 0);
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
		IF_(check() < 0);
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

	PC_GRID_CELL *_PCgrid::getCell(const vInt3 &vC)
	{
		// IF_N(!m_vX.bInside(vC.x));
		// IF_N(!m_vY.bInside(vC.y));
		// IF_N(!m_vZ.bInside(vC.z));

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

	void _PCgrid::getActiveCellLines(LineSet *pLS, int cIdx)
	{
		NULL_(pLS);
		IF_(cIdx >= PCGRID_ACTIVECELL_N);

		// TODO: lock needed?
		mutexLock();
		*pLS = m_pActiveCells[cIdx].m_LS;
		mutexUnlock();
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

}
