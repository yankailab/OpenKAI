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
	DetectorBase();

	m_pClassMgr = NULL;

	m_pContourFrame = NULL;
	m_pSaliencyFrame = NULL;
//	m_pFrame = NULL;

	m_pGMat = NULL;
//	m_pGray = NULL;

	m_bOneImg = 1;

	m_pFrame = new CamFrame();

}

_ObjectDetector::~_ObjectDetector()
{
}

bool _ObjectDetector::init(JSON* pJson)
{
	this->setTargetFPS(30.0);
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
//	m_pFrame = new CamFrame();

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
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		findObjectByContour();
//		findObjectBySaliency();

		this->autoFPSto();
	}

}

void _ObjectDetector::updateFrame(CamFrame* pFrame, CamFrame* pGray)
{
	if(pFrame==NULL)return;
	if(pFrame->getGMat()->empty())return;
	if(pGray==NULL)return;
	if(pGray->getGMat()->empty())return;

	m_pFrame->update(pFrame);
	m_pGMat = m_pFrame->getGMat();
	m_pGMat->download(m_Mat);

	m_pGray = pGray->getGMat();

	this->wakeupThread();
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

		m_pClassMgr->addObject(&m_Mat,&boundRect,NULL);
		return;
	}

	return;
//	m_pContourFrame->switchFrame();
	GpuMat* pThr = m_pContourFrame->getGMat();

	m_pCanny->detect(*m_pGray, *pThr);

	// Detect edges using Threshold
//	cuda::threshold(*m_pGray, *pThr, 200, 255, THRESH_BINARY);
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

		m_pClassMgr->addObject(&m_Mat,&boundRect,&contours_poly[i]);
	}

}

void _ObjectDetector::findObjectBySaliency(void)
{
	//TODO
	if (m_pSaliency->computeSaliency(m_Mat, m_saliencyMat))
	{
		StaticSaliencySpectralResidual spec;
		spec.computeBinaryMap(m_saliencyMat, m_binMat);

		return;
//		m_pSaliencyFrame->switchFrame();
		m_pSaliencyFrame->update(&m_saliencyMat);
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

}

