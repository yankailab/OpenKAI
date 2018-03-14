/*
 * _filterSaturation.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__filterSaturation_H_
#define OpenKAI_src_Data_Augment__filterSaturation_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "_FilterBase.h"

namespace kai
{

class _filterSaturation: public _FilterBase
{
public:
	_filterSaturation();
	~_filterSaturation();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_filterSaturation*) This)->update();
		return NULL;
	}

public:
	double	m_dSaturation;

};
}

#endif
