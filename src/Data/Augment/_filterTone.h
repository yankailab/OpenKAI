/*
 * _filterTone.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterTone_H_
#define OpenKAI_src_Data_Augment__filterTone_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterTone: public _FilterBase
{
public:
	_filterTone();
	~_filterTone();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterTone*) This)->update();
		return NULL;
	}

public:

};
}

#endif
