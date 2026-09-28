/*
 * DataStreamBase.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "DataStreamBase.h"

namespace kai
{

	DataStreamBase::DataStreamBase()
	{
	}

	DataStreamBase::~DataStreamBase()
	{
	}

	void DataStreamBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

	void DataStreamBase::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;

		string cmd;
		IF_(!jKv(j, "cmd", cmd));
	}

}
