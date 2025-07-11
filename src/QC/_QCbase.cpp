/*
 * _QCbase.cpp
 *
 *  Created on: Jul 11, 2025
 *      Author: yankai
 */

#include "_QCbase.h"

namespace kai
{

	_QCbase::_QCbase()
	{
	}

	_QCbase::~_QCbase()
	{
	}

	int _QCbase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

///		pK->v("", &);

		return OK_OK;
	}

	int _QCbase::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		return OK_OK;
	}

	int _QCbase::check(void)
	{
		return _ModuleBase::check();
	}

	void _QCbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}
