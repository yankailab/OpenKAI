/*
 * _FrameCutOut.h
 *
 *  Created on: Nov 16, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Video__FrameCutOut_H_
#define OpenKAI_src_Data_Video__FrameCutOut_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "../_DataBase.h"

namespace kai
{

class _FrameCutOut: public _DataBase
{
public:
	_FrameCutOut();
	~_FrameCutOut();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_FrameCutOut*) This)->update();
		return NULL;
	}

public:
	double m_progress;

	Frame* m_pFrameIn;
	Frame* m_pFrameOut;
};
}

#endif
