/*
 * _filterBilateralBlur.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterBilateralBlur_H_
#define OpenKAI_src_Data_Augment__filterBilateralBlur_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterBilateralBlur: public _FilterBase
{
public:
	_filterBilateralBlur();
	~_filterBilateralBlur();

	bool init(void* pKiss);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterBilateralBlur*) This)->update();
		return NULL;
	}

public:
};
}

#endif
