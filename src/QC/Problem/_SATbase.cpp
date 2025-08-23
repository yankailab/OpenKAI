/*
 * _SATbase.cpp
 *
 *  Created on: Jul 11, 2025
 *      Author: yankai
 */

#include "_SATbase.h"

namespace kai
{

	_SATbase::_SATbase()
	{
	}

	_SATbase::~_SATbase()
	{
	}

	int _SATbase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

///		pK->v("", &);

		return OK_OK;
	}

	int _SATbase::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		return OK_OK;
	}

	int _SATbase::check(void)
	{
		return _ModuleBase::check();
	}

	void _SATbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}
