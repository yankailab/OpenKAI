/*
 * _filterNoise.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterNoise_H_
#define OpenKAI_src_Data_Augment__filterNoise_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterNoise: public _FilterBase
{
public:
	_filterNoise();
	~_filterNoise();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterNoise*) This)->update();
		return NULL;
	}

public:

};
}

#endif
