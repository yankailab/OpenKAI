/*
 * _InRange.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__InRange_H_
#define OpenKAI_src_Vision__InRange_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#include "../_VisionBase.h"

namespace kai
{

class _InRange: public _VisionBase
{
public:
	_InRange();
	virtual ~_InRange();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_InRange*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
	Frame m_fIn;
	double	m_rFrom;
	double	m_rTo;

};

}
#endif
#endif
