/*
 * _Augment.h
 *
 *  Created on: Oct 12, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__Augment_H_
#define OpenKAI_src_Data_Augment__Augment_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "../_DataBase.h"
#include "_FilterBase.h"
#include "_filterBilateralBlur.h"
#include "_filterBlur.h"
#include "_filterBrightness.h"
#include "_filterContrast.h"
#include "_filterCrop.h"
#include "_filterFlip.h"
#include "_filterGaussianBlur.h"
#include "_filterHistEqualize.h"
#include "_filterLowResolution.h"
#include "_filterMedianBlur.h"
#include "_filterNoise.h"
#include "_filterRotate.h"
#include "_filterSaturation.h"
#include "_filterShrink.h"
#include "_filterTone.h"

#define AUGMENT_N_THREAD 128

namespace kai
{

class _Augment: public _DataBase
{
public:
	_Augment();
	~_Augment();

	bool init(void* pKiss);
	bool start(void);

private:
	_FilterBase* createFilterThread(string& filter);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Augment*) This)->update();
		return NULL;
	}

public:
	int	m_nThread;
	_FilterBase* m_ppFB[AUGMENT_N_THREAD];

	vector<string> m_vFilter;
	bool m_bConvertFormat;

};
}

#endif
