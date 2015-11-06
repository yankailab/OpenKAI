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
				m_pStream[i].m_pObjects[j].m_prob[k] = 0;
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

	m_classifier.setup(modelFile, trainedFile, meanFile, labelFile, NUM_DETECT_BATCH);

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

	//OpenCV Saliency
	string saliencyAlgorithm;
	string saliencyTrainPath;

	CHECK_ERROR(pJson->getVal("SALIENCY_ALGORITHM", &saliencyAlgorithm));
	CHECK_ERROR(pJson->getVal("SALIENCY_TRAIN_PATH", &saliencyTrainPath));

//	m_pSaliency = cv::saliency::Saliency::create(saliencyAlgorithm);
	m_pSaliency = ObjectnessBING::create(saliencyAlgorithm);

	if(m_pSaliency == NULL)
	{
		LOG(INFO)<<"Saliency Algorithm not found";
	}
	else
	{
		m_pSaliency.dynamicCast<ObjectnessBING>()->setTrainingPath( saliencyTrainPath );
		m_pSaliency.dynamicCast<ObjectnessBING>()->setBBResDir( saliencyTrainPath + "/Results" );
	}

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



    if(m_pSaliency)
    	{
    		if(m_pSaliency->computeSaliency(*pMat, m_pSaliencyMap ))
    		{


    		}
    }




//	cuda::bilateralFilter(*pGray, m_pGMat, 5, 50, 7);
//	m_pCanny->detect(*pGray, m_pGMat);
//	m_pGMat.download(m_frame);

	//Test
/*	pCS->m_pDenseFlow->detect(pCS->m_pGrayL);

	GpuMat gMat1,gMat2;
	if(pCS->m_pDenseFlow->m_flowMat.empty())return;
	cuda::abs(pCS->m_pDenseFlow->m_flowMat,gMat1);

	vector<GpuMat> fMat(2);
	cuda::split(gMat1,fMat);
	cuda::add(fMat[0],fMat[1],gMat1);
	cuda::threshold(gMat1, gMat2, 5, 255, THRESH_BINARY);

	gMat2.download(m_frame);
	m_frame.convertTo(matThr,CV_8UC1);
*/


	int i,j;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Mat matThr;

	GpuMat gThr;
	// Detect edges using Threshold
	cuda::threshold(*pGray, gThr, 50, 255, THRESH_BINARY);
	gThr.download(m_frame);

	// Find contours
	findContours(m_frame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
//	findContours(m_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// Approximate contours to polygons + get bounding rects
	vector<vector<Point> > contours_poly(contours.size());
	Rect boundRect;

	pDS->m_numObj = 0;
	for (i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);

		boundRect = boundingRect(Mat(contours_poly[i]));
		if(boundRect.area()<5000)continue;

		int extraW = boundRect.width*0.15;
		int extraH = boundRect.height*0.15;

		boundRect.x -= extraW;
		boundRect.y -= extraH;
		if(boundRect.x<0)boundRect.x=0;
		if(boundRect.y<0)boundRect.y=0;

		boundRect.width += extraW+extraW;
		boundRect.height += extraH+extraH;
		int overW = m_frame.cols - boundRect.x - boundRect.width;
		int overH = m_frame.rows - boundRect.y - boundRect.height;
		if(overW<0)boundRect.width+=overW;
		if(overH<0)boundRect.height+=overH;

		pDS->m_pObjects[pDS->m_numObj].m_boundBox = boundRect;
		pCS->m_pFrameL->m_uFrame(boundRect).copyTo(pDS->m_pObjects[pDS->m_numObj].m_pImg);
		pDS->m_numObj++;
	}


	vector<Mat> vImg;
	NN_OBJECT* pObj;

/*
	for (i = 0; i < pDS->m_numObj; i++)
	{
		pObj = &pDS->m_pObjects[i];

		vImg.clear();
		vImg.push_back(pObj->m_pImg);
		m_vPredictions = m_classifier.ClassifyBatch(vImg,5);

		m_predictions = m_vPredictions[0];
		for (j = 0; j < m_predictions.size(); j++)
		{
			Prediction p = m_predictions[j];

			pObj->m_name[j] = p.first;
			pObj->m_prob[j] = p.second;

			int from = pObj->m_name[j].find_first_of(' ');
			int to = pObj->m_name[j].find_first_of(' ');
			pObj->m_name[j] = pObj->m_name[j].substr(from+1,pObj->m_name[j].length());

			if (j >= NUM_OBJECT_NAME)break;
		}
	}

	return;
*/




	if(pDS->m_numObj>NUM_DETECT_BATCH)pDS->m_numObj = NUM_DETECT_BATCH;

	for (i = 0; i < pDS->m_numObj; i++)
	{
		pObj = &pDS->m_pObjects[i];
		vImg.push_back(pObj->m_pImg);
	}

	m_vPredictions = m_classifier.ClassifyBatch(vImg,5);//pDS->m_numObj);

	for (i = 0; i < pDS->m_numObj; i++)
//	for (i = 0; i < m_vPredictions.size(); i++)
	{
		pObj = &pDS->m_pObjects[i];
		for (j = 0; j < m_vPredictions[i].size(); j++)
		{
			Prediction p = m_vPredictions[i][j];

			pObj->m_name[j] = p.first;
			pObj->m_prob[j] = p.second;

			int from = pObj->m_name[j].find_first_of(' ');
			int to = pObj->m_name[j].find_first_of(' ');
			pObj->m_name[j] = pObj->m_name[j].substr(from+1,pObj->m_name[j].length());

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

