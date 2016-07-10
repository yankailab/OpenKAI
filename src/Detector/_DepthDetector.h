/*
 *
 */


#ifndef DETECTOR_DEPTHDETECTOR_H_
#define DETECTOR_DEPTHDETECTOR_H_

#include "../AI/_Classifier.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "DetectorBase.h"
#include "../Camera/_CamStream.h"
#include "../Camera/CamStereo.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

namespace kai
{

class _DepthDetector: public DetectorBase, public _ThreadBase
{
public:
	_DepthDetector();
	~_DepthDetector();

	bool init(JSON* pJson, string camName);
	bool start(void);

private:
	void detect(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DepthDetector*) This)->update();
		return NULL;
	}

public:
	_Classifier* m_pClassifier;

	CamFrame*	m_pGray;
	CamFrame*	m_pDepth;
	uint64_t	m_camFrameID;

	Mat			m_contourMat;
	Mat			m_Mat;

	_CamStream*	m_pCamStream;
	CamInput*	m_pCam;
	CamFrame*	m_pFrame;
	CamStereo*	m_pStereo;

	//Classifier
//	NNClassifier m_classifier;
//	vector<Prediction> m_predictions;
//	vector<vector<Prediction> > m_vPredictions;

};
}

#endif /* SRC_DEPTHDETECTOR_H_ */
