/*
 * ObjectLocalizer.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "ObjectDetector.h"
namespace kai
{
ObjectDetector::ObjectDetector()
{
	m_bThreadON = false;
	m_threadID = 0;
	m_numObjDetected = 0;
	m_frameID = 0;
	m_processedFrameID = 0;
	m_numImg = 0;

}

ObjectDetector::~ObjectDetector()
{
}

bool ObjectDetector::init(JSON* pJson)
{
	string modelFile;
	string trainedFile;
	string meanFile;
	string labelFile;

	CHECK_FATAL(pJson->getVal("CAFFE_MODEL_FILE", &modelFile));
	CHECK_FATAL(pJson->getVal("CAFFE_TRAINED_FILE", &trainedFile));
	CHECK_FATAL(pJson->getVal("CAFFE_MEAN_FILE", &meanFile));
	CHECK_FATAL(pJson->getVal("CAFFE_LABEL_FILE", &labelFile));

	m_classifier.setup(
			modelFile,
			trainedFile,
			meanFile,
			labelFile
			);

	LOG(INFO)<<"Object Detector Initialized";

	return true;
}

bool ObjectDetector::start(void)
{
	m_tSleep = TRD_INTERVAL_OBJDETECTOR;
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

		detect();

		if(m_processedFrameID == m_frameID)
		{
			this->sleepThread(0,m_tSleep);//sleepThread can be woke up by this->wakeupThread()
		}
	}

}

void ObjectDetector::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);

	LOG(INFO) << "ObjectDetector.stop()";
}

void ObjectDetector::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

bool ObjectDetector::complete(void)
{
	return true;
}

void ObjectDetector::setFrame(Mat img)
{
	if(++m_frameID == MAX_FRAME_ID)
	{
		m_frameID = 0;
	}

	m_frame = img;

	this->wakeupThread();
}

int  ObjectDetector::getObject(NN_OBJECT** ppObjects)
{
	*ppObjects = m_pObjects;
	return m_numObjDetected;
}

void ObjectDetector::detect(void)
{
	if(m_frame.rows+m_frame.cols==0)return;


	size_t i;
	m_predictions = m_classifier.Classify(m_frame);
	m_processedFrameID = m_frameID;

	for (i = 0; i < m_predictions.size(); i++)
	{
		Prediction p = m_predictions[i];

		m_pObjects[i].m_name[0] = p.first;
		if(i>=NUM_OBJECTS)
		{
			break;
		}
	}
	m_numObjDetected = i;
}

}




