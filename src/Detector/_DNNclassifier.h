/*
 * _DNNclassifier.h
 *
 *  Created on: Jan 13, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__DNNclassifier_H_
#define OpenKAI_src_DNN_Darknet__DNNclassifier_H_

#include "../Base/common.h"
#include "../Vision/_VisionBase.h"
#include "../Detector/_DetectorBase.h"

namespace kai
{

class _DNNclassifier: public _DetectorBase
{
public:
	_DNNclassifier();
	~_DNNclassifier();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);
	bool classify(Mat m, OBJECT* pO);

private:
	bool classify(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DNNclassifier*) This)->update();
		return NULL;
	}

public:
	cv::dnn::Net m_net;
	vector<vFloat4>	m_vROI;
	int		m_nW;
	int		m_nH;
	bool	m_bSwapRB;
	float	m_scale;
	vInt3	m_vMean;
	Mat		m_blob;

	int m_iBackend;
	int m_iTarget;

};

}
#endif
