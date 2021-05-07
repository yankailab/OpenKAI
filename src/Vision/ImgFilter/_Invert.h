/*
 * _Invert.h
 *
 *  Created on: March 14, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Invert_H_
#define OpenKAI_src_Vision__Invert_H_

#ifdef USE_OPENCV
#include "../_VisionBase.h"

namespace kai
{

class _Invert: public _VisionBase
{
public:
	_Invert();
	virtual ~_Invert();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Invert*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
	Frame m_fIn;

};

}
#endif
#endif
