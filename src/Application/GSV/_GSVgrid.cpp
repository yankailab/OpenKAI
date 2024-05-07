#include "_GSVgrid.h"

namespace kai
{

	_GSVgrid::_GSVgrid()
	{
		m_type = pc_grid;
	}

	_GSVgrid::~_GSVgrid()
	{
		DEL(m_pCell);
	}

	bool _GSVgrid::init(void *pKiss)
	{
		IF_F(!this->_PCgrid::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return initBuffer();
	}

	int _GSVgrid::check(void)
	{
		return this->_PCgrid::check();
	}

}
