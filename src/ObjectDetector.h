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

using namespace cv;
using namespace cv::cuda;

#define TRD_INTERVAL_OBJDETECTOR 10000
#define NUM_OBJECT_NAME 5
#define NUM_OBJECTS 1000
#define NUM_IMG 100

namespace kai
{

struct NN_OBJECT
{
	string m_name[NUM_OBJECT_NAME];
	iVector4 m_posSize;
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

	void setFrame(Mat pImg);
	int  getObject(NN_OBJECT** ppObjects);

private:
	void detect(void);

private:
	int m_frameID;
	int m_processedFrameID;
	Mat m_frame;

	int m_numImg;
	Mat m_pImg[NUM_IMG];

	int m_numObjDetected;
	NN_OBJECT m_pObjects[NUM_OBJECTS];

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
