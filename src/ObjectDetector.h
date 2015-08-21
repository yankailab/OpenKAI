/*
 * ObjectLocalizer.h
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#ifndef SRC_OBJECTDETECTOR_H_
#define SRC_OBJECTDETECTOR_H_

#include "common.h"
#include "NNClassifier.h"

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/core/ocl.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui.hpp"

#ifdef USE_CUDA
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudaoptflow.hpp"
#include "opencv2/cudaarithm.hpp"
#endif

namespace kai
{
using namespace cv;
using namespace cv::cuda;

#define TRD_INTERVAL 1
#define NUM_OBJECT_NAME 5
#define NUM_OBJECTS 1000

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

	int detect(Mat pImg, NN_OBJECT** ppObjects);

private:
	pthread_t m_threadID;
	bool m_bThreadON;

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((ObjectDetector *) This)->update();
		return NULL;
	}

	int m_numObjDetected;
	NN_OBJECT m_pObjects[NUM_OBJECTS];
	NNClassifier m_classifier;
	std::vector<Prediction> m_predictions;

};
}

#endif /* SRC_OBJECTDETECTOR_H_ */
