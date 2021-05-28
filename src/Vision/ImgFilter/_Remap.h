/*
 * _Remap.h
 *
 *  Created on: May 7, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Remap_H_
#define OpenKAI_src_Vision__Remap_H_

#ifdef USE_OPENCV
#include "../_VisionBase.h"
#include "../../IO/_File.h"

namespace kai
{

class _Remap: public _VisionBase
{
public:
	_Remap();
	virtual ~_Remap();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);
	bool bReady(void);

private:
	void filter(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_Remap*) This)->update();
		return NULL;
	}

public:
	_VisionBase* m_pV;
	bool m_bReady;
};

}
#endif
#endif
