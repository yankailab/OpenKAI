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

	int i;
	for(i=0;i<NUM_DETECTOR_STREAM;i++)
	{
		m_pStream[i].m_frameID = 0;
		m_pStream[i].m_numImg = 0;
		m_pStream[i].m_pCamStream = NULL;
	}

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
	int i;
	CamFrame* pFrame;
	int tThreadBegin;
	DETECTOR_STREAM* pDS;
	m_tSleep = TRD_INTERVAL_OBJDETECTOR;

	while (m_bThreadON)
	{
		tThreadBegin = time(NULL);

		for(i=0;i<NUM_DETECTOR_STREAM;i++)
		{
			pDS = &m_pStream[i];
			if(!pDS->m_pCamStream)continue;

			pFrame = *(pDS->m_pCamStream->m_pFrameProcess);
			if(pDS->m_frameID == pFrame->m_frameID)continue;

			detect(i);
		}

		this->sleepThread(0,m_tSleep);//sleepThread can be woke up by this->wakeupThread()
	}

}

void ObjectDetector::setFrame(int iStream, CamStream* pCam)
{
	if(!pCam)return;

	DETECTOR_STREAM* pDS = &m_pStream[iStream];
	pDS->m_pCamStream = pCam;
//	this->wakeupThread();
}

int  ObjectDetector::getObject(int iStream, NN_OBJECT** ppObjects)
{
	DETECTOR_STREAM* pDS = &m_pStream[iStream];
	*ppObjects = pDS->m_pObjects;
	return pDS->m_numImg;
}

void ObjectDetector::detect(int iStream)
{
	DETECTOR_STREAM* pDS = &m_pStream[iStream];
	CamFrame* pFrame = *(pDS->m_pCamStream->m_pFrameProcess);

	if(pFrame->m_uFrame.empty())return;

	pDS->m_frameID = pFrame->m_frameID;
	pFrame->m_uFrame.copyTo(pDS->m_pImg[0]);

	if(pDS->m_pImg[0].empty())return;

	m_predictions = m_classifier.Classify(pDS->m_pImg[0]);//TODO:change to separated local images


	size_t i;
	for (i = 0; i < m_predictions.size(); i++)
	{
		Prediction p = m_predictions[i];

		pDS->m_pObjects[0].m_name[i] = p.first;
		if(i>=NUM_OBJECT_NAME)break;
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



}




