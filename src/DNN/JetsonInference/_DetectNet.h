/*
 * _DetectNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_TensorRT__DetectNet_H_
#define OpenKAI_src_DNN_TensorRT__DetectNet_H_

#include "../../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_JETSON_INFERENCE
#include "../../Vision/_VisionBase.h"
#include "../../Detector/_DetectorBase.h"

namespace kai
{

class _DetectNet: public _DetectorBase
{
public:
	_DetectNet();
	~_DetectNet();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	bool draw(void);

private:
	bool open(void);
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DetectNet*) This)->update();
		return NULL;
	}

public:
	detectNet* m_pDN;
	int m_nBox;
	uint32_t m_nClass;

	Frame* m_pRGBA;
	Frame* m_pRGBAf;
	float m_coverageThr;
	bool	m_bSwapRB;
	vFloat3 m_vMean;

};

}
#endif
#endif
#endif
