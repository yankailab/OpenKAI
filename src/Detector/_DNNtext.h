/*
 * _DNNtext.h
 *
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__DNNtext_H_
#define OpenKAI_src_DNN_Darknet__DNNtext_H_

#include "_DNNdetect.h"

namespace kai
{

class _DNNtext: public _DNNdetect
{
public:
	_DNNtext();
	~_DNNtext();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	int check(void);

private:
	void decode(const Mat& mScores, const Mat& mGeometry, float scoreThresh,
	        std::vector<RotatedRect>& vDetections, std::vector<float>& vConfidences);
	bool detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DNNtext*) This)->update();
		return NULL;
	}

public:
	vDouble3 m_vMean;

};

}
#endif
