/*
 * ClassifierManager.cpp
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#include "ClassifierManager.h"

namespace kai
{

ClassifierManager::ClassifierManager()
{
	m_bThreadON = false;
	m_threadID = 0;

	int i, j;
	m_numObj = 0;
	m_numBatch = 0;
	m_globalFrameID = 0;

	for (i = 0; i < NUM_OBJ; i++)
	{
		m_pObjects[i].m_frameID = 0;
		m_pObjects[i].m_classifyBefore = 0;
		m_pObjects[i].m_status = OBJ_VACANT;

		for (j = 0; j < NUM_OBJECT_NAME; j++)
		{
			m_pObjects[i].m_name[j] = "";
			m_pObjects[i].m_prob[j] = 0;
		}
	}

}

ClassifierManager::~ClassifierManager()
{
	// TODO Auto-generated destructor stub
}

bool ClassifierManager::init(JSON* pJson)
{
	//Setup Caffe Classifier
	string modelFile;
	string trainedFile;
	string meanFile;
	string labelFile;

	CHECK_FATAL(pJson->getVal("CAFFE_MODEL_FILE", &modelFile));
	CHECK_FATAL(pJson->getVal("CAFFE_TRAINED_FILE", &trainedFile));
	CHECK_FATAL(pJson->getVal("CAFFE_MEAN_FILE", &meanFile));
	CHECK_FATAL(pJson->getVal("CAFFE_LABEL_FILE", &labelFile));

	m_classifier.setup(modelFile, trainedFile, meanFile, labelFile, NUM_DETECT_BATCH);

	LOG(INFO)<<"Caffe Initialized";

	return true;
}

bool ClassifierManager::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void ClassifierManager::update(void)
{
	m_tSleep = TRD_INTERVAL_CLASSIFIER_MANAGER;

	while (m_bThreadON)
	{
		this->updateTime();

		m_globalFrameID = get_time_usec();

		classifyObject();

		//sleepThread can be woke up by this->wakeupThread()
		this->sleepThread(0, m_tSleep);
	}

}

void ClassifierManager::classifyObject(void)
{
	int i, j;
	vector<Mat> vImg;
	OBJECT* pObj;
	int numBatch;
	OBJECT* pObjBatch[NUM_DETECT_BATCH];

	vImg.clear();
	numBatch = 0;
	for (i = 0; i < m_numObj; i++)
	{
		pObj = &m_pObjects[i];

		if(pObj->m_status == OBJ_VACANT)continue;
		if(pObj->m_status == OBJ_COMPLETE)continue;

		if(pObj->m_classifyBefore > m_globalFrameID)
		{
			pObj->m_status = OBJ_VACANT;
			continue;
		}

		//TODO: resize
		pObj->m_status = OBJ_CLASSIFYING;
		pObjBatch[numBatch] = pObj;
		vImg.push_back(pObj->m_Mat);

		numBatch++;
		if(numBatch >= NUM_DETECT_BATCH)break;
	}

	if(numBatch <= 0)return;

	//Get the top 5 possible labels
	m_vPredictions = m_classifier.ClassifyBatch(vImg, 5);

	for (i = 0; i < numBatch; i++)
	{
		pObj = pObjBatch[i];

		for (j = 0; j < m_vPredictions[i].size(); j++)
		{
			Prediction p = m_vPredictions[i][j];

			pObj->m_name[j] = p.first;
			pObj->m_prob[j] = p.second;

			int from = pObj->m_name[j].find_first_of(' ');
			int to = pObj->m_name[j].find_first_of(' ');

			pObj->m_name[j] = pObj->m_name[j].substr(from + 1, pObj->m_name[j].length());

			if (j >= NUM_OBJECT_NAME)break;
		}

		pObj->m_status = OBJ_COMPLETE;
	}
}

bool ClassifierManager::addObject(uint64_t frameID, Mat* pMat, Rect* pRect)
{
	if(!pMat)return false;
	if(!pRect)return false;

	int i;
	OBJECT* pObj;

	for(i=0; i<NUM_OBJ; i++)
	{
		pObj = &m_pObjects[i];
		if(pObj->m_status != OBJ_VACANT)continue;

		pObj->m_status = OBJ_ADDED;
		pObj->m_frameID = frameID;
		pObj->m_boundBox = *pRect;
		pMat->copyTo(pObj->m_Mat);
		return true;
	}

	return false;
}


void ClassifierManager::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);
}

void ClassifierManager::waitForComplete(void)
{
	pthread_join(m_threadID, NULL);
}

bool ClassifierManager::complete(void)
{
	return true;
}


} /* namespace kai */
