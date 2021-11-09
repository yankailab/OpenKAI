#include "_PClattice.h"

#ifdef USE_OPEN3D

namespace kai
{

_PClattice::_PClattice()
{
	m_type = pc_lattice;
}

_PClattice::~_PClattice()
{
}

bool _PClattice::init(void *pKiss)
{
	IF_F(!this->_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

//	pK->v("v", &m_v);

	return true;
}

bool _PClattice::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _PClattice::check(void)
{
	return this->_PCbase::check();
}

void _PClattice::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		render();

		m_pT->autoFPSto();
	}
}

void _PClattice::render(void)
{
}

}
#endif
