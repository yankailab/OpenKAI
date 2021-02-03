/*
 * _Contrast.h
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Contrast_H_
#define OpenKAI_src_Vision__Contrast_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#include "../_VisionBase.h"

namespace kai
{

class _Contrast: public _VisionBase
{
public:
	_Contrast();
	virtual ~_Contrast();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Contrast*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
	double m_alpha;
	double m_beta;
};

}
#endif
#endif
