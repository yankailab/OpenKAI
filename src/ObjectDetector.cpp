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

	int i,j,k;
	for (i = 0; i < NUM_DETECTOR_STREAM; i++)
	{
		m_pStream[i].m_frameID = 0;
		m_pStream[i].m_numObj = 0;
		m_pStream[i].m_pCamStream = NULL;

		for(j=0;j<NUM_OBJ;j++)
		{
			for(k=0;k<NUM_OBJECT_NAME;k++)
			{
				m_pStream[i].m_pObjects[j].m_name[k] = "";
			}
		}
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

	m_classifier.setup(modelFile, trainedFile, meanFile, labelFile);

	LOG(INFO)<<"Object Detector Initialized";

	int lowThr = 10;
	int highThr = 100;
	int Apperture = 3;
	CHECK_ERROR(pJson->getVal("OD_CANNY_LOW", &lowThr));
	CHECK_ERROR(pJson->getVal("OD_CANNY_HIGH", &highThr));
	CHECK_ERROR(pJson->getVal("OD_CANNY_APPERTURE", &Apperture));
	m_pCanny = cuda::createCannyEdgeDetector(lowThr, highThr, Apperture);
/*
	int ksize = 3;
	int sigma = 1;
	CHECK_ERROR(pJson->getVal("OD_GAUSS_KSIZE", &ksize));
	CHECK_ERROR(pJson->getVal("OD_GAUSS_SIGMA", &sigma));
	m_pGaussian = cuda::createGaussianFilter(CV_8U, CV_8U, ksize, sigma);
*/
	return true;
}

bool ObjectDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode << " in ObjectDetector::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "ObjectDetector.start()";

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

		for (i = 0; i < NUM_DETECTOR_STREAM; i++)
		{
			pDS = &m_pStream[i];
			if (!pDS->m_pCamStream)
				continue;

			pFrame = *(pDS->m_pCamStream->m_pFrameProcess);

			//The current frame is the latest frame
			if (pDS->m_frameID == pFrame->m_frameID)
				continue;

			detect(i);
		}

		//sleepThread can be woke up by this->wakeupThread()
		this->sleepThread(0, m_tSleep);
	}

}

void ObjectDetector::detect(int iStream)
{
	DETECTOR_STREAM* pDS = &m_pStream[iStream];
	CamStream* pCS = pDS->m_pCamStream;
	CamFrame* pFrame = *(pCS->m_pFrameProcess);
	Mat* pMat = &pFrame->m_uFrame;

	if (pMat->empty())
		return;

	pDS->m_frameID = pFrame->m_frameID;
	GpuMat* pGray = pCS->m_pGrayL->m_pNext;

	if (pGray->empty())return;

//	cuda::bilateralFilter(*pGray, m_pGMat, 5, 50, 7);
//	m_pCanny->detect(*pGray, m_pGMat);
//	m_pGMat.download(m_frame);

	pGray->download(m_frame);

	int i,j;
	int thresh = 100;
	int max_thresh = 255;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Mat matThr;

	// Detect edges using Threshold
	cv::threshold(m_frame, matThr, thresh, 255, THRESH_BINARY);
	// Find contours
	findContours(matThr, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	// Approximate contours to polygons + get bounding rects
	vector<vector<Point> > contours_poly(contours.size());
	Rect boundRect;

	pDS->m_numObj = 0;
	for (i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);

		boundRect = boundingRect(Mat(contours_poly[i]));
		if(boundRect.area()<10000)continue;

		boundRect.x *= 0.9;
		boundRect.y *= 0.9;

		boundRect.width *= 1.25;
		boundRect.height *= 1.25;
		if(boundRect.x + boundRect.width > m_frame.cols)
		{
			boundRect.width = m_frame.cols - boundRect.x;
		}
		if(boundRect.y + boundRect.height > m_frame.rows)
		{
			boundRect.height = m_frame.rows - boundRect.y;
		}

		pDS->m_pObjects[pDS->m_numObj].m_boundBox = boundRect;
		pCS->m_pFrameL->m_uFrame(boundRect).copyTo(pDS->m_pObjects[pDS->m_numObj].m_pImg);
		pDS->m_numObj++;
	}


	NN_OBJECT* pObj;
	for (i = 0; i < pDS->m_numObj; i++)
	{
		pObj = &pDS->m_pObjects[i];
		m_predictions = m_classifier.Classify(pObj->m_pImg);

		for (j = 0; j < m_predictions.size(); j++)
		{
			Prediction p = m_predictions[j];

			pObj->m_name[j] = p.first;
			if (j >= NUM_OBJECT_NAME)break;
		}

	}

//	imshow("Canny", m_frame);return;


	//TODO:Perform a quick HOG detection on people
	//TODO:Perform a quick HOG detection on car

}

void ObjectDetector::setFrame(int iStream, CamStream* pCam)
{
	if (!pCam)
		return;

	DETECTOR_STREAM* pDS = &m_pStream[iStream];
	pDS->m_pCamStream = pCam;
//	this->wakeupThread();
}

int ObjectDetector::getObject(int iStream, NN_OBJECT** ppObjects)
{
	DETECTOR_STREAM* pDS = &m_pStream[iStream];
	*ppObjects = pDS->m_pObjects;
	return pDS->m_numObj;
}

void ObjectDetector::stop(void)
{
	m_bThreadON = false;
	this->wakeupThread();
	pthread_join(m_threadID, NULL);

	LOG(INFO)<< "ObjectDetector.stop()";
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

