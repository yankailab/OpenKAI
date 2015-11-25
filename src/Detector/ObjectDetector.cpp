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

	int i, j;
	m_frameID = 0;
	m_numObj = 0;
	m_pCamStream = NULL;

	for (i = 0; i < NUM_OBJ; i++)
	{
		for (j = 0; j < NUM_OBJECT_NAME; j++)
		{
			m_pObjects[i].m_name[j] = "";
			m_pObjects[i].m_prob[j] = 0;
		}
	}

}

ObjectDetector::~ObjectDetector()
{
}

bool ObjectDetector::init(JSON* pJson)
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

	m_classifier.setup(modelFile, trainedFile, meanFile, labelFile,
	NUM_DETECT_BATCH);

	LOG(INFO)<<"Caffe Initialized";

	//OpenCV Canny Edge Detector
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

	m_pSaliency = Saliency::create(saliencyAlgorithm);
//	m_pSaliency = ObjectnessBING::create(saliencyAlgorithm);

	if (m_pSaliency == NULL)
	{
		LOG(INFO)<<"Saliency Algorithm not found";
	}
	else
	{
//		m_pSaliency.dynamicCast<ObjectnessBING>()->setTrainingPath( saliencyTrainPath );
//		m_pSaliency.dynamicCast<ObjectnessBING>()->setBBResDir( saliencyTrainPath + "/Results" );
	}

	m_pContourFrame = new CamFrame();
	m_pContourFrame->init();
	m_pSaliencyFrame = new CamFrame();
	m_pSaliencyFrame->init();

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
	m_tSleep = TRD_INTERVAL_OBJDETECTOR;

	while (m_bThreadON)
	{
		tThreadBegin = time(NULL);

		if (!m_pCamStream)
			continue;
		pFrame = *(m_pCamStream->m_pFrameProcess);

		//The current frame is not the latest frame
		if (m_frameID != pFrame->m_frameID)
		{
			m_frameID = pFrame->m_frameID;
			detect();
		}

		//sleepThread can be woke up by this->wakeupThread()
		this->sleepThread(0, m_tSleep);
	}

}

void ObjectDetector::detect(void)
{
	m_pFrame = *(m_pCamStream->m_pFrameProcess);
	m_pGMat = m_pFrame->getCurrentFrame();

	if (m_pGMat->empty())return;
	m_pGMat->download(m_Mat);

	m_pGray = m_pCamStream->m_pGrayL->getCurrentFrame();
	if (m_pGray->empty())return;

	m_numObj = 0;

	findObjectByContour();
//	findObjectBySaliency();
	classifyObject();

}

void ObjectDetector::classifyObject(void)
{
	int i, j;
	vector<Mat> vImg;
	OBJECT* pObj;

	if (m_numObj > NUM_DETECT_BATCH)
		m_numObj = NUM_DETECT_BATCH;

	for (i = 0; i < m_numObj; i++)
	{
		vImg.push_back(m_pObjects[i].m_pImg);
	}

	m_vPredictions = m_classifier.ClassifyBatch(vImg, 5);

	for (i = 0; i < m_numObj; i++)
	{
		pObj = &m_pObjects[i];
		for (j = 0; j < m_vPredictions[i].size(); j++)
		{
			Prediction p = m_vPredictions[i][j];

			pObj->m_name[j] = p.first;
			pObj->m_prob[j] = p.second;

			int from = pObj->m_name[j].find_first_of(' ');
			int to = pObj->m_name[j].find_first_of(' ');
			pObj->m_name[j] = pObj->m_name[j].substr(from + 1,
					pObj->m_name[j].length());

			if (j >= NUM_OBJECT_NAME)
				break;
		}
	}
}

void ObjectDetector::findObjectByContour(void)
{
	int i, j;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	m_pContourFrame->switchFrame();
	GpuMat* pThr = m_pContourFrame->getCurrentFrame();

	// Detect edges using Threshold
	cuda::threshold(*m_pGray, *pThr, 50, 255, THRESH_BINARY);
	pThr->download(m_contourMat);

	// Find contours
	findContours(m_contourMat, contours, hierarchy, CV_RETR_TREE,
			CV_CHAIN_APPROX_SIMPLE);
	//	findContours(m_frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// Approximate contours to polygons + get bounding rects
	vector<vector<Point> > contours_poly(contours.size());
	Rect boundRect;

	for (i = 0; i < contours.size(); i++)
	{
		if(m_numObj>=NUM_OBJ)break;

		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);

		boundRect = boundingRect(Mat(contours_poly[i]));
		if (boundRect.area() < 5000)
			continue;

		int extraW = boundRect.width * 0.15;
		int extraH = boundRect.height * 0.15;

		boundRect.x -= extraW;
		boundRect.y -= extraH;
		if (boundRect.x < 0)
			boundRect.x = 0;
		if (boundRect.y < 0)
			boundRect.y = 0;

		boundRect.width += extraW + extraW;
		boundRect.height += extraH + extraH;

		int overW = m_Mat.cols - boundRect.x - boundRect.width;
		int overH = m_Mat.rows - boundRect.y - boundRect.height;
		if (overW < 0)
			boundRect.width += overW;
		if (overH < 0)
			boundRect.height += overH;

		m_pObjects[m_numObj].m_boundBox = boundRect;
		m_Mat(boundRect).copyTo(m_pObjects[m_numObj].m_pImg);
		m_numObj++;
	}

}

void ObjectDetector::findObjectBySaliency(void)
{
	if (m_pSaliency->computeSaliency(m_Mat, m_saliencyMat))
	{
		StaticSaliencySpectralResidual spec;
		spec.computeBinaryMap(m_saliencyMat, m_binMat);
	}

/*
	if (m_pSaliency->computeSaliency(*pMat, m_pSaliencyMap))
	{
		Rect boundRect;
		Vec4i iRect;

		j = m_pSaliencyMap.size();
		if (j > NUM_OBJ)
			j = NUM_OBJ;

		pDS->m_numObj = 0;
		for (i = 0; i < j; i++)
		{
			if(m_numObj>=NUM_OBJ)break;

			iRect = m_pSaliencyMap[i];
			boundRect.x = iRect[0];
			boundRect.y = iRect[1];
			boundRect.width = iRect[2];
			boundRect.height = iRect[3];

			pDS->m_pObjects[pDS->m_numObj].m_boundBox = boundRect;
			pDS->m_pObjects[pDS->m_numObj].m_name[0] = "test";
			pDS->m_numObj++;
		}

	}
*/
}

void ObjectDetector::findObjectByOpticalFlow(void)
{
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

}

void ObjectDetector::setCamStream(CamStream* pCam)
{
	if (!pCam)
		return;
	m_pCamStream = pCam;
}

int ObjectDetector::getObject(OBJECT** ppObjects)
{
	*ppObjects = m_pObjects;
	return m_numObj;
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

