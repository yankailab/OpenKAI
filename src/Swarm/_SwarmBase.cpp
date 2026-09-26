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

	bool _SwarmBase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());

//		jKv(j, "tExpire", m_tExpire);

		return true;
	}

	bool _SwarmBase::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		return true;
	}

	bool _SwarmBase::check(void)
	{
		return this->_ModuleBase::check();
	}


	void _SwarmBase::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(!check());
		this->_ModuleBase::console(pConsole);
		
	}

}
