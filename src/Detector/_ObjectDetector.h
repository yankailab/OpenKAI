
#ifndef DETECTOR_OBJECTDETECTOR_H_
#define DETECTOR_OBJECTDETECTOR_H_

#include "../AI/_ClassifierManager.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"
//#include "../Camera/_CamStream.h"
#include "DetectorBase.h"

using namespace cv;
using namespace cv::cuda;
using namespace cv::saliency;
using namespace std;

#define TRD_INTERVAL_OBJDETECTOR 1000


namespace kai
{

class _ObjectDetector: public DetectorBase, public _ThreadBase
{
public:
	_ObjectDetector();
	~_ObjectDetector();

	bool init(JSON* pJson);
	bool start(void);

//	void setCamStream(_CamStream* pCam);

	void updateFrame(CamFrame* pFrame, CamFrame* pGray);

private:
	void findObjectByContour(void);
	void findObjectByOpticalFlow(void);
	void findObjectBySaliency(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ObjectDetector*) This)->update();
		return NULL;
	}

public:

	//Temporal demo
	int			m_bOneImg;

	_ClassifierManager* m_pClassMgr;

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
//	NNClassifier m_classifier;
//	vector<Prediction> m_predictions;
//	vector<vector<Prediction> > m_vPredictions;

};
}

#endif /* SRC_OBJECTDETECTOR_H_ */
