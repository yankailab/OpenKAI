#include "_PCgrid.h"
#include "_PCstream.h"

namespace kai
{

	_PCgrid::_PCgrid()
	{
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
	}

	_PCgrid::~_PCgrid()
	{
		DEL(m_pCell);
	}

	bool _PCgrid::init(void *pKiss)
	{
		IF_F(!this->_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
		pK->m_pInst = this;

		pK->v("vPorigin", &m_vPorigin);
		pK->v("vX", &m_vX);
		pK->v("vY", &m_vY);
		pK->v("vZ", &m_vZ);
		pK->v("vCellSize", &m_vCellSize);

		return initBuffer();
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

		m_vRx.set(m_vPorigin.x - m_vX.x * m_vCellSize.x,
				  m_vPorigin.x + m_vX.y * m_vCellSize.x);

		m_vRy.set(m_vPorigin.y - m_vY.x * m_vCellSize.y,
				  m_vPorigin.y + m_vY.y * m_vCellSize.y);

		m_vRz.set(m_vPorigin.z - m_vZ.x * m_vCellSize.z,
				  m_vPorigin.z + m_vZ.y * m_vCellSize.z);

		m_vPmin.set(m_vRx.x, m_vRy.x, m_vRz.x);

		m_vOvCellSize.set(1.0 / m_vCellSize.x,
						  1.0 / m_vCellSize.y,
						  1.0 / m_vCellSize.z);

		m_dYZ = m_vDim.y * m_vDim.z;

		return true;
	}

	void _PCgrid::clear(void)
	{
		for (int i = 0; i < m_nCell; i++)
			m_pCell[i].clear();
	}

	void _PCgrid::getStream(void *p, const uint64_t &tExpire)
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

	int _PCgrid::check(void)
	{
		return this->_GeometryBase::check();
	}

}
