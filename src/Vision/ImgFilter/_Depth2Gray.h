/*
 * _Depth2Gray.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Depth2Gray_H_
#define OpenKAI_src_Vision__Depth2Gray_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#include "../_DepthVisionBase.h"

namespace kai
{

class _Depth2Gray: public _VisionBase
{
public:
	_Depth2Gray();
	virtual ~_Depth2Gray();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Depth2Gray*) This)->update();
		return NULL;
	}

public:
	_DepthVisionBase* m_pV;
	Frame m_fIn;

};

}
#endif
#endif
