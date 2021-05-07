/*
 * _FrameCutOut.h
 *
 *  Created on: Nov 16, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Video__FrameCutOut_H_
#define OpenKAI_src_Data_Video__FrameCutOut_H_

#include "../_DataBase.h"
#include "../../Vision/Frame.h"

#ifdef USE_OPENCV

namespace kai
{

class _FrameCutOut: public _DataBase
{
public:
	_FrameCutOut();
	~_FrameCutOut();

	bool init(void* pKiss);
	bool start(void);

private:
	void update(void);
	static void* getUpdate(void* This)
	{
		((_FrameCutOut*) This)->update();
		return NULL;
	}

public:
	double m_progress;

	Frame m_frameIn;
	Frame m_frameOut;
};

}
#endif
#endif
