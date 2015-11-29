/*
 * ObjectLocalizer.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef DETECTOR_OBJECTDETECTOR_H_
#define DETECTOR_OBJECTDETECTOR_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../AI/ClassifierManager.h"
#include "../AI/NNClassifier.h"
#include "../Camera/CamStream.h"
#include "DetectorBase.h"

using namespace cv;
using namespace cv::cuda;
using namespace cv::saliency;
using namespace std;

#define TRD_INTERVAL_OBJDETECTOR 0
#define NUM_DETECT_BATCH 10

namespace kai
{
/*
struct OBJECT
{
	string 		m_name[NUM_OBJECT_NAME];
	double		m_prob[NUM_OBJECT_NAME];
	Mat         m_pImg;
	Rect			m_boundBox;
};
*/
class ObjectDetector: public DetectorBase, ThreadBase
{
public:
	ObjectDetector();
	~ObjectDetector();

	bool init(JSON* pJson);
	bool start(void);
	bool complete(void);
	void stop(void);
	void waitForComplete(void);

	void setCamStream(CamStream* pCam);
	int  getObject(OBJECT** ppObjects);

private:
	void detect(void);
	void classifyObject(void);
	void findObjectByContour(void);
	void findObjectByOpticalFlow(void);
	void findObjectBySaliency(void);

	pthread_t m_threadID;
	bool m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((ObjectDetector*) This)->update();
		return NULL;
	}

public:

	//Temporal demo
	int			m_bOneImg;



	CamStream*	m_pCamStream;
	OBJECT 		m_pObjects[NUM_OBJ];
	int 			m_numObj;

	CamFrame*	m_pContourFrame;
	CamFrame*	m_pSaliencyFrame;
	GpuMat* 		m_pGMat;
	GpuMat*		m_pGray;

	Mat			m_contourMat;
	Mat			m_saliencyMat;
	Mat			m_binMat;
	Mat			m_Mat;

	//OpenCV Saliency algorithms
	Ptr<cuda::CannyEdgeDetector> m_pCanny;
//	Ptr<cuda::> m_pGaussian;
	Ptr<Saliency> m_pSaliency;
	vector<Vec4i> m_pSaliencyMap;

	//Caffe classifier
	NNClassifier m_classifier;
	vector<Prediction> m_predictions;
	vector<vector<Prediction> > m_vPredictions;

};
}

#endif /* SRC_OBJECTDETECTOR_H_ */
