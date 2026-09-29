/*
 * UGLIDcellStream.cpp
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#include "UGLIDcellStream.h"

namespace kai
{

	UGLIDcellStream::UGLIDcellStream()
	{
	}

	UGLIDcellStream::~UGLIDcellStream()
	{
	}

	void UGLIDcellStream::console(void *pConsole)
	{
		NULL_(pConsole);
		DataObjBase::console(pConsole);
	}

}
