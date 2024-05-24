#include "_GSVgrid.h"

namespace kai
{

	_GSVgrid::_GSVgrid()
	{
	}

	_GSVgrid::~_GSVgrid()
	{
	}

	bool _GSVgrid::init(void *pKiss)
	{
		IF_F(!this->_PCgrid::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;



		return true;
	}

	bool _GSVgrid::link(void)
	{
		IF_F(!this->_PCgrid::link());
		Kiss *pK = (Kiss *)m_pKiss;


		return true;
	}

    bool _GSVgrid::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

	int _GSVgrid::check(void)
	{
		return this->_PCgrid::check();
	}

	void _GSVgrid::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateGSVgrid();

			m_pT->autoFPSto();
		}
	}

	void _GSVgrid::updateGSVgrid(void)
	{
		IF_(check() < 0);

		clear();

		for(_GeometryBase* pG : m_vpGB)
		{
			addGeometry(pG, m_tExpire);
		}

		updateActiveCell();
	}


}
