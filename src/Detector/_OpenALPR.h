/*
 * _OpenALPR.h
 *
 *  Created on: March 19, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Detector__OpenALPR_H_
#define OpenKAI_src_DNN_Detector__OpenALPR_H_

#include "../Base/common.h"
#include "../Vision/_VisionBase.h"
#include "_DetectorBase.h"

#ifdef USE_OPENALPR
#include <alpr.h>

namespace kai
{

class _OpenALPR: public _DetectorBase
{
public:
	_OpenALPR();
	~_OpenALPR();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	int check(void);

private:
	bool detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_OpenALPR*) This)->update();
		return NULL;
	}

public:
	alpr::Alpr* m_pAlpr;

};

}
#endif
#endif
