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

#define TRD_INTERVAL_OBJDETECTOR 10000
#define NUM_OBJECT_NAME 5
#define NUM_IMG 100
#define NUM_DETECTOR_STREAM 5

namespace kai
{

struct NN_OBJECT
{
	string 		m_name[NUM_OBJECT_NAME];
	iVector4 	m_posSize;
};

struct DETECTOR_STREAM
{
	CamStream*	m_pCamStream;

	int m_frameID;
	int m_numImg;
	Mat m_pImg[NUM_IMG];
	NN_OBJECT m_pObjects[NUM_IMG];
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

	void setFrame(int iStream, CamStream* pCam);
	int  getObject(int iStream, NN_OBJECT** ppObjects);

private:
	void detect(int iStream);

private:
	DETECTOR_STREAM m_pStream[NUM_DETECTOR_STREAM];
	NNClassifier m_classifier;
	std::vector<Prediction> m_predictions;

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
