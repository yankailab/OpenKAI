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
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

//	pK->v("v", &m_v);

	return true;
}

bool _Crystal::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _Crystal::check(void)
{
	return 0;
}

void _Crystal::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		render();

		this->autoFPSto();
	}
}

void _Crystal::render(void)
{
}

void _Crystal::draw(void)
{
	this->_ThreadBase::draw();
}

}
#endif
