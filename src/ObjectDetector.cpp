/*
 * ObjectLocalizer.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "ObjectDetector.h"

ObjectDetector::ObjectDetector()
{
	// TODO Auto-generated constructor stub

}

ObjectDetector::~ObjectDetector()
{
	// TODO Auto-generated destructor stub
}

bool ObjectDetector::init(void)
{

	m_classifier.setup(
			"/Users/yankai/Documents/dev/caffe-master/models/bvlc_reference_caffenet/deploy.prototxt", //model_file,
			"/Users/yankai/Documents/dev/caffe-master/models/bvlc_reference_caffenet/bvlc_reference_caffenet.caffemodel", //trained_file,
			"/Users/yankai/Documents/dev/caffe-master/data/ilsvrc12/imagenet_mean.binaryproto", //mean_file,
			"/Users/yankai/Documents/dev/caffe-master/data/ilsvrc12/synset_words.txt" //label_file
			);

	m_tSleep = TRD_INTERVAL;

	LOG(INFO)<<"Object Detector Initialized";

	return true;
}

bool ObjectDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode << " in ObjectDetector::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO) << "ObjectDetector.start()";

	return true;
}

void ObjectDetector::update(void)
{
	int tThreadBegin;

	while (m_bThreadON)
	{
		tThreadBegin = time(NULL);

		this->sleepThread(m_tSleep,0);//sleepThread can be woke up by this->wakeupThread()
	}

}

void ObjectDetector::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);

//	m_pVController->stop();

	LOG(INFO) << "ObjectDetector.stop()";
}

void ObjectDetector::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

int ObjectDetector::detect(Mat pImg, NN_OBJECT** ppObjects)
{
	size_t i;
	m_predictions = m_classifier.Classify(pImg);

	for (i = 0; i < m_predictions.size(); i++)
	{
		Prediction p = m_predictions[i];

		m_pObjects[i].m_name[0] = p.first;
		if(i>=NUM_OBJECTS)
		{
			break;
		}
	}

	*ppObjects = m_pObjects;
	return i;
}



