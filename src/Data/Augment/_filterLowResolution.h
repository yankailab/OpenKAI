/*
 * _filterLowResolution.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterLowResolution_H_
#define OpenKAI_src_Data_Augment__filterLowResolution_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterLowResolution: public _FilterBase
{
public:
	_filterLowResolution();
	~_filterLowResolution();

	bool init(void* pKiss);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterLowResolution*) This)->update();
		return NULL;
	}

public:
	double m_minScale;

};
}

#endif
