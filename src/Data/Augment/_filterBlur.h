/*
 * _filterBlur.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterBlur_H_
#define OpenKAI_src_Data_Augment__filterBlur_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterBlur: public _FilterBase
{
public:
	_filterBlur();
	~_filterBlur();

	bool init(void* pKiss);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterBlur*) This)->update();
		return NULL;
	}

public:

};
}

#endif
