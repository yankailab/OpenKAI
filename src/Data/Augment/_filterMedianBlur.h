/*
 * _filterMedianBlur.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterMedianBlur_H_
#define OpenKAI_src_Data_Augment__filterMedianBlur_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterMedianBlur: public _FilterBase
{
public:
	_filterMedianBlur();
	~_filterMedianBlur();

	bool init(void* pKiss);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterMedianBlur*) This)->update();
		return NULL;
	}

public:

};
}

#endif
