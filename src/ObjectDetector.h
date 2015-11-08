/*
 * ObjectLocalizer.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef SRC_OBJECTDETECTOR_H_
#define SRC_OBJECTDETECTOR_H_

#include "common.h"
#include "cvplatform.h"
#include "NNClassifier.h"

#include "CamStream.h"

using namespace cv;
using namespace cv::cuda;
using namespace cv::saliency;
using namespace std;

#define TRD_INTERVAL_OBJDETECTOR 0
#define NUM_OBJECT_NAME 5
#define NUM_OBJ 100
#define NUM_DETECT_BATCH 10

namespace kai
{

struct OBJECT
{
	string 		m_name[NUM_OBJECT_NAME];
	double		m_prob[NUM_OBJECT_NAME];
	Mat         m_pImg;
	Rect			m_boundBox;
};

class ObjectDetector: public ThreadBase
{
public:
	ObjectDetector();
	~ObjectDetector();

	bool init(JSON* pJson);
	bool start(void);
	bool complete(void);
	void stop(void);
	void waitForComplete(void);

	void setFrame(CamStream* pCam);
	int  getObject(OBJECT** ppObjects);

private:
	void detect(void);

public:
	int 			m_frameID;
	CamStream*	m_pCamStream;
	OBJECT 		m_pObjects[NUM_OBJ];
	int 			m_numObj;

	Mat		m_frame;
	Mat		m_binMap;
	Mat		m_saliencyMap;
	GpuMat  m_pGMat;

	//OpenCV algorithms
	Ptr<cuda::CannyEdgeDetector> m_pCanny;
//	Ptr<cuda::> m_pGaussian;
	Ptr<Saliency> m_pSaliency;
	vector<Vec4i> m_pSaliencyMap;

	//Caffe classifier
	NNClassifier m_classifier;
	vector<Prediction> m_predictions;
	vector<vector<Prediction> > m_vPredictions;

private:
	pthread_t m_threadID;
	bool m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((ObjectDetector *) This)->update();
		return NULL;
	}


};
}

#endif /* SRC_OBJECTDETECTOR_H_ */
