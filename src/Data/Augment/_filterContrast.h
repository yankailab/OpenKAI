/*
 * _filterContrast.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterContrast_H_
#define OpenKAI_src_Data_Augment__filterContrast_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterContrast: public _FilterBase
{
public:
	_filterContrast();
	~_filterContrast();

	bool init(void* pKiss);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterContrast*) This)->update();
		return NULL;
	}

public:

};
}

#endif
