#include "_Crystal.h"

#ifdef USE_OPEN3D

namespace kai
{

_Crystal::_Crystal()
{
}

_Crystal::~_Crystal()
{
}

bool _Crystal::init(void *pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

//	pK->v("v", &m_v);

	return true;
}

bool _Crystal::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _Crystal::check(void)
{
	return this->_ModuleBase::check();
}

void _Crystal::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		render();

		m_pT->autoFPSto();
	}
}

void _Crystal::render(void)
{
}

void _Crystal::draw(void)
{
	this->_ModuleBase::draw();
}

}
#endif
