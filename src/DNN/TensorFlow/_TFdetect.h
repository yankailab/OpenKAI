/*
 * _TFdetect.h
 *
 *  Created on: Aug 29, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_TensorFlow__TFdetect_H_
#define OpenKAI_src_DNN_TensorFlow__TFdetect_H_

#include "../../Base/common.h"
#include "../../Vision/_VisionBase.h"
#include "../../Base/_ObjectBase.h"

#ifdef USE_TENSORFLOW

namespace kai
{

class _TFdetect: public _ObjectBase
{
public:
	_TFdetect();
	~_TFdetect();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_TFdetect*) This)->update();
		return NULL;
	}

public:

	Frame m_BGR;
};

}

#endif
#endif
