/*
 * _DNNtext.h
 *
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__DNNtext_H_
#define OpenKAI_src_DNN_Darknet__DNNtext_H_

#include "_DNNdetect.h"

#ifdef USE_OPENCV
#include "OCR.h"

namespace kai
{

class _DNNtext: public _DNNdetect
{
public:
	_DNNtext();
	~_DNNtext();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);
	void ocr(void);

public:
	Mat getTransform(Mat mImg, vFloat2* pBox);
	void decode(const Mat& mScores, const Mat& mGeometry, float scoreThresh,
	        std::vector<RotatedRect>& vDetections, std::vector<float>& vConfidences);
	void detect(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_DNNtext*) This)->update();
		return NULL;
	}

public:
	vDouble3 m_vMean;
	bool	m_bDetect;
	bool	m_bOCR;
	bool	m_bWarp;

#ifdef USE_OCR
	OCR* m_pOCR;
#endif

};

}
#endif
#endif
