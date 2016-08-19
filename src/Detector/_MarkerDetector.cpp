/*
 * CameraMarkerDetect.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_MarkerDetector.h"

namespace kai
{

_MarkerDetector::_MarkerDetector()
{
	_ThreadBase();
	DetectorBase();

	m_cudaDeviceID = 0;

	m_numCircle = 0;
	m_minMarkerSize = 0;
	m_areaRatio = 0;

	m_pCamStream = NULL;
}

_MarkerDetector::~_MarkerDetector()
{
	// TODO Auto-generated destructor stub
}


bool _MarkerDetector::init(JSON* pJson, string name)
{
	double FPS = DEFAULT_FPS;

	CHECK_INFO(pJson->getVal("MARKER_" + name + "_FPS", &FPS));
	CHECK_ERROR(pJson->getVal("MARKER_" + name + "_AREA_RATIO", &m_areaRatio));
	CHECK_ERROR(pJson->getVal("MARKER_" + name + "_MIN_SIZE", &m_minMarkerSize));

	this->setTargetFPS(FPS);
	return true;
}

bool _MarkerDetector::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _MarkerDetector::update(void)
{
	cuda::setDevice(m_cudaDeviceID);

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detectCircle();

		this->autoFPSto();
	}

}

void _MarkerDetector::detectCircle(void)
{
	int i,j,k;
	Point2f center;
	float radius;
	vector< vector< Point > > contours;
	vector<Vec3f> circles;
	fVector4 v4tmp;
	Mat matThresh;
	Frame* pHSV;
	Frame* pRGB;

	if(!m_pCamStream)return;
	pHSV = m_pCamStream->getHSVFrame();
	pRGB = m_pCamStream->getFrame();
	if(pRGB->empty())return;
	if(pHSV->empty())return;

	vector<GpuMat> matHSV(3);
	matHSV[0] = m_Hue;
	matHSV[1] = m_Sat;
	matHSV[2] = m_Val;

	cuda::split(*(pHSV->getGMat()), matHSV);

	m_Hue = matHSV[0];
	m_Sat = matHSV[1];
	m_Val = matHSV[2];

	//Filtering the image
	cuda::absdiff(m_Hue, Scalar(90), m_Huered);
	cuda::multiply(m_Huered, Scalar(0.25), m_Scalehuered);	//1/4
	cuda::multiply(m_Sat, Scalar(0.0625), m_Scalesat);	//1/16
	cuda::multiply(m_Scalehuered, m_Scalesat, m_Balloonyness);
	cuda::threshold(m_Balloonyness, m_Thresh, 200, 255, THRESH_BINARY);
//	cuda::threshold(m_Balloonyness, m_Thresh, 200, 255, THRESH_BINARY_INV);
	m_Thresh.download(matThresh);

	//Find the contours
	findContours(matThresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
//		drawContours(pRGB->m_uFrame, contours, -1, Scalar(255, 0, 0));

	//Find marker
	m_numCircle = 0;
	for (i=0; i<contours.size(); i++)
	{
		minEnclosingCircle(contours[i], center, radius);

//			circle(pRGB->m_uFrame, center, radius, Scalar(0, 255, 0), 1);

		//New marker found
		if (contourArea(contours[i]) < m_areaRatio*radius*radius*3.1415926)continue;
		if (radius < m_minMarkerSize)continue;

//			circle(pRGB->m_uFrame, center, radius, Scalar(0, 255, 0), 2);

		m_pCircle[m_numCircle].m_x = center.x;
		m_pCircle[m_numCircle].m_y = center.y;
		m_pCircle[m_numCircle].m_r = radius;
		m_numCircle++;

		if (m_numCircle == NUM_MARKER)
		{
			break;
		}
	}
}

bool _MarkerDetector::getCircleCenter(fVector3* pCenter)
{
	Camera*	pCam;

	if(pCenter==NULL)return false;
	if(m_pCamStream==NULL)return false;

	pCam = m_pCamStream->getCameraInput();
	if(pCam==NULL)return false;

	//Use num instead of m_numCircle to avoid multi-thread inconsistancy
	int num = m_numCircle;
	if(num==0)return false;

	int i;
	MARKER_CIRCLE* pMarker = &m_pCircle[0];
	MARKER_CIRCLE* pCompare;
	int camCenter = (pCam->m_width+pCam->m_height)/2;

	//Find the closest point
	for(i=1; i<num; i++)
	{
		pCompare = &m_pCircle[i];
		if(abs(pCompare->m_x + pCompare->m_y - camCenter) < abs(pMarker->m_x + pMarker->m_y - camCenter))
		{
			pMarker = pCompare;
		}
	}

	pCenter->m_x = pMarker->m_x;
	pCenter->m_y = pMarker->m_y;
	pCenter->m_z = pMarker->m_r;

	return true;
}


} /* namespace kai */
