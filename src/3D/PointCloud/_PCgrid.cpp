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
		m_vCellSize.clear();
		m_vPorigin.clear();
	}

	_PCgrid::~_PCgrid()
	{
		DEL(m_pCell);
	}

	bool _PCgrid::init(void *pKiss)
	{
		IF_F(!this->_GeometryBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vDim", &m_vDim);
		pK->v("vCellSize", &m_vCellSize);
		pK->v("vPorigin", &m_vPorigin);

		return initBuffer();
	}

	bool _PCgrid::initBuffer(void)
	{
		int nCell = m_vDim.x * m_vDim.y * m_vDim.z;
		IF_F(nCell <= 0);

		float cVol = m_vCellSize.x * m_vCellSize.y * m_vCellSize.z;
		IF_F(cVol <= 0);

		DEL(m_pCell);
		m_pCell = new PC_GRID_CELL[nCell];
		NULL_F(m_pCell);
		m_nCell = nCell;

		return true;
	}

	void _PCgrid::getStream(void* p)
	{
		NULL_(p);

		_PCstream* pS = (_PCstream*)p;

		

	}

	int _PCgrid::check(void)
	{
		return this->_GeometryBase::check();
	}

}
