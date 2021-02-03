/*
 * _Crop.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Crop_H_
#define OpenKAI_src_Vision__Crop_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#include "../_VisionBase.h"

namespace kai
{

class _Crop: public _VisionBase
{
public:
	_Crop();
	virtual ~_Crop();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Crop*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
	Frame m_fIn;

	vFloat4 m_roi;

};

}
#endif
#endif
