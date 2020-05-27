/*
 * _Cascade.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__Cascade_H_
#define OpenKAI_src_Detector__Cascade_H_

#include "../Detector/_DetectorBase.h"

#ifdef USE_OPENCV
#ifdef USE_CASCADE
#ifdef USE_CUDA

namespace kai
{

class _Cascade: public _DetectorBase
{
public:
	_Cascade();
	~_Cascade();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

private:
	void detectCPU(void);
	void detectGPU(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Cascade*) This)->update();
		return NULL;
	}

public:
	cv::CascadeClassifier	m_CC;
	Ptr<cuda::CascadeClassifier> m_pGCC;

	bool m_bGPU;
	double m_minSize;
	double m_maxSize;
	double m_scaleFactor;
	int m_minNeighbors;
	vDouble4 m_area;

	string m_className;

};

}
#endif
#endif
#endif
#endif
