/*
 * ObjectLocalizer.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_ObjectDetector.h"

namespace kai
{
_ObjectDetector::_ObjectDetector()
{
	_ThreadBase();

	m_pClassMgr = NULL;
	m_pCamStream = NULL;

}

_ObjectDetector::~_ObjectDetector()
{
}

bool _ObjectDetector::init(JSON* pJson)
{
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
	m_pSaliencyFrame = new CamFrame();
	m_pFrame = new CamFrame();

	return true;
}

bool _ObjectDetector::start(void)
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

void _ObjectDetector::update(void)
{
	CamFrame* pFrame;
	m_tSleep = TRD_INTERVAL_OBJDETECTOR;

	while (m_bThreadON)
	{
		this->updateTime();

		if (!m_pCamStream)continue;
		pFrame = *(m_pCamStream->m_pFrameProcess);

		//The current frame is not the latest frame
		if (pFrame->isNewerThan(m_pFrame))
		{
			m_pFrame->updateFrame(pFrame);
			detect();
		}

		//sleepThread can be woke up by this->wakeupThread()
		this->sleepThread(0, m_tSleep);
	}

}

void _ObjectDetector::detect(void)
{
	m_pGMat = m_pFrame->getCurrentFrame();

	if (m_pGMat->empty())return;
	m_pGMat->download(m_Mat);

	m_pGray = m_pCamStream->m_pGrayL->getCurrentFrame();
	if (m_pGray->empty())return;

	findObjectByContour();
//	findObjectBySaliency();
}

void _ObjectDetector::findObjectByContour(void)
{
	int i;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Rect boundRect;

	//DEMO
	if(m_bOneImg==1)
	{
		boundRect.height = m_Mat.size().height - 50;
		boundRect.width = boundRect.height;
		boundRect.x = (m_Mat.size().width - boundRect.width)*0.5;
		boundRect.y = (m_Mat.size().height - boundRect.height)*0.5;

		m_pClassMgr->addObject(get_time_usec(),&m_Mat,&boundRect);
		return;
	}

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

	for (i = 0; i < contours.size(); i++)
	{
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

		m_pClassMgr->addObject(get_time_usec(),&m_Mat,&boundRect);
	}

}

void _ObjectDetector::findObjectBySaliency(void)
{
	if (m_pSaliency->computeSaliency(m_Mat, m_saliencyMat))
	{
		StaticSaliencySpectralResidual spec;
		spec.computeBinaryMap(m_saliencyMat, m_binMat);
	}
}

void _ObjectDetector::findObjectByOpticalFlow(void)
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

void _ObjectDetector::setCamStream(_CamStream* pCam)
{
	if (!pCam)
		return;
	m_pCamStream = pCam;
}

}

