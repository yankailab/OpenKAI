/*
 * _HistEqualize.h
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__HistEqualize_H_
#define OpenKAI_src_Vision__HistEqualize_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#include "../_VisionBase.h"

namespace kai
{

class _HistEqualize: public _VisionBase
{
public:
	_HistEqualize();
	virtual ~_HistEqualize();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_HistEqualize*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
};

}
#endif
#endif
