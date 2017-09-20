/*
 * _DetectNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_DNN__DetectNet_H_
#define OPENKAI_SRC_DNN__DetectNet_H_

#include "../../Base/common.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Vision/_VisionBase.h"

#ifdef USE_TENSORRT
#include "detectNet.h"
#include "cudaMappedMemory.h"

namespace kai
{

class _DetectNet: public _DetectorBase
{
public:
	_DetectNet();
	~_DetectNet();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
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
	int m_nBoxMax;
	uint32_t m_nClass;

	int num_channels_;
	cv::Mat mean_;
	Frame* m_pRGBA;
	Frame* m_pRGBAf;
	double m_coverageThr;

	double m_minSize;
	double m_maxSize;
	vDouble4 m_area;

	float* m_bbCPU;
	float* m_bbCUDA;
	float* m_confCPU;
	float* m_confCUDA;

	string m_className;

};

}

#endif
#endif
