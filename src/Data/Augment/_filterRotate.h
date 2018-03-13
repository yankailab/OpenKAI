/*
 * _filterRotate.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterRotate_H_
#define OpenKAI_src_Data_Augment__filterRotate_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterRotate: public _FilterBase
{
public:
	_filterRotate();
	~_filterRotate();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterRotate*) This)->update();
		return NULL;
	}

public:

};
}

#endif
