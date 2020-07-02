/*
 * _Mask.h
 *
 *  Created on: July 2, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Mask_H_
#define OpenKAI_src_Vision__Mask_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#include "../_VisionBase.h"

namespace kai
{

class _Mask: public _VisionBase
{
public:
	_Mask();
	virtual ~_Mask();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Mask*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
	_VisionBase* m_pVmask;

	Frame m_fIn;
	Frame m_fMask;

};

}
#endif
#endif
