/*
 * _MeshBase.cpp
 *
 *  Created on: April 3, 2023
 *      Author: yankai
 */

#include "_MeshBase.h"

namespace kai
{

	_MeshBase::_MeshBase()
	{
	}

	_MeshBase::~_MeshBase()
	{
	}

	bool _MeshBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		//		pK->v("shmName", &m_shmName);

		IF_F(!open());

		return true;
	}

	bool _MeshBase::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		return true;
	}

	int _MeshBase::check(void)
	{
		return this->_StateBase::check();
	}

	void _MeshBase::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(check() < 0);
		this->_ModuleBase::console(pConsole);

		// string msg = "id=" + i2str(pO->getTopClass());
		// ((_Console *)pConsole)->addMsg(msg, 1);
	}

}
