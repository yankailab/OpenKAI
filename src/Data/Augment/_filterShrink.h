/*
 * _filterShrink.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterShrink_H_
#define OpenKAI_src_Data_Augment__filterShrink_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterShrink: public _FilterBase
{
public:
	_filterShrink();
	~_filterShrink();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterShrink*) This)->update();
		return NULL;
	}

public:

};
}

#endif
