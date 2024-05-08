#include "_PCgrid.h"

namespace kai
{

	_PCgrid::_PCgrid()
	{
		m_pS = NULL;

		m_type = pc_grid;
		m_pCell = NULL;
		m_nCell = 0;
		m_vDim.clear();
		m_vPmin.clear();

		m_vPorigin.clear();
		m_vX.clear();
		m_vY.clear();
		m_vZ.clear();
		m_vCellSize.clear();

		m_bVisual = false;
		m_gridLine.Clear();
		m_gLineWidth = 1;
		m_vAxisColX.set(1, 0, 0);
		m_vAxisColY.set(0, 1, 0);
		m_vAxisColZ.set(0, 0, 1);

		m_hlcLineWidth = 5;
		m_vHLClineCol.set(1,1,1);
	}

	_PCgrid::~_PCgrid()
	{
		DEL(m_pCell);
	}

	bool _PCgrid::init(void *pKiss)
	{
		IF_F(!this->_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vPorigin", &m_vPorigin);
		pK->v("vX", &m_vX);
		pK->v("vY", &m_vY);
		pK->v("vZ", &m_vZ);
		pK->v("vCellSize", &m_vCellSize);

		pK->v("bVisual", &m_bVisual);
		pK->v("gLineWidth", &m_gLineWidth);
		pK->v("vAxisColX", &m_vAxisColX);
		pK->v("vAxisColY", &m_vAxisColY);
		pK->v("vAxisColZ", &m_vAxisColZ);
		pK->v("hlcLineWidth", &m_hlcLineWidth);
		pK->v("vHLClineCol", &m_vHLClineCol);

		return initBuffer();
	}

	bool _PCgrid::link(void)
	{
		IF_F(!this->_GeometryBase::link());

		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_PCstream", &n);
		m_pS = (_PCstream *)(pK->getInst(n));

		return true;
	}

	int _PCgrid::check(void)
	{
		NULL__(m_pS, -1);

		return this->_GeometryBase::check();
	}

	bool _PCgrid::initBuffer(void)
	{
		float cVol = m_vCellSize.x * m_vCellSize.y * m_vCellSize.z;
		IF_F(cVol <= 0);

		m_vDim.set(m_vX.len(),
				   m_vY.len(),
				   m_vZ.len());
		int nCell = m_vDim.x * m_vDim.y * m_vDim.z;
		IF_F(nCell <= 0);

		DEL(m_pCell);
		m_pCell = new PC_GRID_CELL[nCell];
		NULL_F(m_pCell);
		m_nCell = nCell;

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

			vInt3 vC = {0, 0, 0};
			m_vHLCidx.push_back(vC);
			generateHLClines();
		}

		return true;
	}

	void _PCgrid::clear(void)
	{
		for (int i = 0; i < m_nCell; i++)
			m_pCell[i].clear();
	}

	PC_GRID_CELL *_PCgrid::getCell(const vFloat3 &vP)
	{
		IF_N(!m_vRx.bInside(vP.x));
		IF_N(!m_vRy.bInside(vP.y));
		IF_N(!m_vRz.bInside(vP.z));

		vInt3 vPi;
		vPi.x = (vP.x - m_vPmin.x) * m_vOvCellSize.x;
		vPi.y = (vP.y - m_vPmin.y) * m_vOvCellSize.y;
		vPi.z = (vP.z - m_vPmin.z) * m_vOvCellSize.z;

		return getCell(vPi);
	}

	PC_GRID_CELL *_PCgrid::getCell(const vInt3 &vPi)
	{
		int i = vPi.x * m_dYZ + vPi.y * m_vDim.z + vPi.z;

		return &m_pCell[i];
	}

	LineSet *_PCgrid::getGridLines(void)
	{
		return &m_gridLine;
	}

	LineSet *_PCgrid::getHLCLines(void)
	{
		return &m_hlcLine;
	}

	void _PCgrid::getPCstream(void *p, const uint64_t &tExpire)
	{
		IF_(check() < 0);
		NULL_(p);
		_PCstream *pS = (_PCstream *)p;

		uint64_t tNow = getApproxTbootUs();
		int nP = pS->nP();

		for (int i = 0; i < nP; i++)
		{
			GEOMETRY_POINT *pP = pS->get(i);
			IF_CONT(bExpired(pP->m_tStamp, tExpire, tNow));

			PC_GRID_CELL *pC = getCell(vFloat3(pP->m_vP[0], pP->m_vP[1], pP->m_vP[2]));
			IF_CONT(!pC);

			pC->add(1);
		}
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


	void _PCgrid::updateHLClines(void)
	{
		m_hlcLine.Clear();
		m_hlcLine.points_.clear();
		m_hlcLine.lines_.clear();
		m_hlcLine.colors_.clear();

		generateHLClines();
	}

	void _PCgrid::generateHLClines(void)
	{
		vFloat3 pO, pA, pB;

		for (vInt3 vC : m_vHLCidx)
		{
			pO.x = m_vRx.x + m_vCellSize.x * vC.x;
			pO.y = m_vRy.x + m_vCellSize.y * vC.y;
			pO.z = m_vRz.x + m_vCellSize.z * vC.z;

			pA = pO;
			pB = pA;
			pB.x += m_vCellSize.x;
			addHLCline(pA, pB, m_vHLClineCol);

			pB = pA;
			pB.y += m_vCellSize.y;
			addHLCline(pA, pB, m_vHLClineCol);

			pB = pA;
			pB.z += m_vCellSize.z;
			addHLCline(pA, pB, m_vHLClineCol);


			pA = pO;
			pA.x += m_vCellSize.x;
			pA.y += m_vCellSize.y;

			pB = pA;
			pB.x -= m_vCellSize.x;
			addHLCline(pA, pB, m_vHLClineCol);

			pB = pA;
			pB.y -= m_vCellSize.y;
			addHLCline(pA, pB, m_vHLClineCol);

			pB = pA;
			pB.z += m_vCellSize.z;
			addHLCline(pA, pB, m_vHLClineCol);


			pA = pO;
			pA.x += m_vCellSize.x;
			pA.z += m_vCellSize.z;

			pB = pA;
			pB.x -= m_vCellSize.x;
			addHLCline(pA, pB, m_vHLClineCol);

			pB = pA;
			pB.y += m_vCellSize.y;
			addHLCline(pA, pB, m_vHLClineCol);

			pB = pA;
			pB.z -= m_vCellSize.z;
			addHLCline(pA, pB, m_vHLClineCol);


			pA = pO;
			pA.y += m_vCellSize.y;
			pA.z += m_vCellSize.z;

			pB = pA;
			pB.x += m_vCellSize.x;
			addHLCline(pA, pB, m_vHLClineCol);

			pB = pA;
			pB.y -= m_vCellSize.y;
			addHLCline(pA, pB, m_vHLClineCol);

			pB = pA;
			pB.z -= m_vCellSize.z;
			addHLCline(pA, pB, m_vHLClineCol);
		}
	}

	void _PCgrid::addHLCline(const vFloat3 &pA,
							 const vFloat3 &pB,
							 const vFloat3 &vCol)
	{
		int nP = m_hlcLine.points_.size();
		m_hlcLine.points_.push_back(Vector3d{pA.x, pA.y, pA.z});
		m_hlcLine.points_.push_back(Vector3d{pB.x, pB.y, pB.z});
		m_hlcLine.lines_.push_back(Vector2i{nP, nP + 1});
		m_hlcLine.colors_.push_back(Vector3d{vCol.x, vCol.y, vCol.z});
	}

}
