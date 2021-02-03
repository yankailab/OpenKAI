/*
 * _MotionDetector.h
 *
 *  Created on: Aug 4, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__MotionDetector_H_
#define OpenKAI_src_Detector__MotionDetector_H_

#include "_DetectorBase.h"

#ifdef USE_OPENCV
#ifdef USE_OPENCV_CONTRIB

namespace kai
{

class _MotionDetector : public _DetectorBase
{
public:
	_MotionDetector();
	virtual ~_MotionDetector();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_MotionDetector*) This)->update();
		return NULL;
	}


public:
	_VisionBase* m_pVision;
	string m_algorithm;
	cv::Ptr<cv::BackgroundSubtractor> m_pBS;
	double m_learningRate;
	Mat m_mFG;

};

}
#endif
#endif
#endif
