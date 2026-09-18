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

	bool _SwarmBase::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

//		jKv(j, "tExpire", m_tExpire);

		return true;
	}

	bool _SwarmBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

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
