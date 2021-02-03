/*
 * _Rotate.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Rotate_H_
#define OpenKAI_src_Vision__Rotate_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#include "../_VisionBase.h"

namespace kai
{

class _Rotate: public _VisionBase
{
public:
	_Rotate();
	virtual ~_Rotate();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Rotate*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
	int m_code;

};

}
#endif
#endif
