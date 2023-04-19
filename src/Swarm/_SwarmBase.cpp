/*
 * _SwarmBase.cpp
 *
 *  Created on: April 3, 2023
 *      Author: yankai
 */

#include "_SwarmBase.h"

namespace kai
{

	_SwarmBase::_SwarmBase()
	{
	}

	_SwarmBase::~_SwarmBase()
	{
	}

	bool _SwarmBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		//		pK->v("shmName", &m_shmName);

		IF_F(!open());

		return true;
	}

	bool _SwarmBase::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		return true;
	}

	int _SwarmBase::check(void)
	{
		return this->_StateBase::check();
	}

	void _SwarmBase::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(check() < 0);
		this->_ModuleBase::console(pConsole);

		// string msg = "id=" + i2str(pO->getTopClass());
		// ((_Console *)pConsole)->addMsg(msg, 1);
	}

}
