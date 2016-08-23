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

	m_method = METHOD_FILL;
	m_numCircle = 0;
	m_minMarkerSize = 0;
	m_areaRatio = 0;
	m_kSize = 1;
	m_houghMinDist = 100;
	m_houghParam1 = 100;
	m_houghParam2 = 20;
	m_houghMinR = 0;
	m_houghMaxR = 0;

	m_pCamStream = NULL;
	m_pFrame = NULL;
}

_MarkerDetector::~_MarkerDetector()
{
	// TODO Auto-generated destructor stub
}


bool _MarkerDetector::init(JSON* pJson, string name)
{
	CHECK_ERROR(pJson->getVal("MARKER_" + name + "_AREA_RATIO", &m_areaRatio));
	CHECK_ERROR(pJson->getVal("MARKER_" + name + "_MIN_SIZE", &m_minMarkerSize));

	CHECK_INFO(pJson->getVal("MARKER_" + name + "_METHOD", &m_method));
	CHECK_INFO(pJson->getVal("MARKER_" + name + "_MEDBLUR_KSIZE", &m_kSize));

	CHECK_INFO(pJson->getVal("MARKER_" + name + "_HOUGH_MINDIST", &m_houghMinDist));
	CHECK_INFO(pJson->getVal("MARKER_" + name + "_HOUGH_PARAM1", &m_houghParam1));
	CHECK_INFO(pJson->getVal("MARKER_" + name + "_HOUGH_PARAM2", &m_houghParam2));
	CHECK_INFO(pJson->getVal("MARKER_" + name + "_HOUGH_MINR", &m_houghMinR));
	CHECK_INFO(pJson->getVal("MARKER_" + name + "_HOUGH_MAXR", &m_houghMaxR));


	double FPS = DEFAULT_FPS;
	CHECK_INFO(pJson->getVal("MARKER_" + name + "_FPS", &FPS));
	this->setTargetFPS(FPS);

	m_pFrame = new Frame();

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

		switch (m_method)
		{
			case METHOD_FILL:
				detectCircleFill();
				break;
			case METHOD_HOUGH:
				detectCircleHough();
				break;
			default:
				detectCircleFill();
				break;
		}

		this->autoFPSto();
	}

}


void _MarkerDetector::detectCircleFill(void)
{
	int i,j;
	Point2f center;
	float radius;
	vector< vector< Point > > contours;
	vector<Vec3f> circles;
	Mat matThresh;
	Frame* pHSV;
	Frame* pRGB;

	if(!m_pCamStream)return;
	pHSV = m_pCamStream->getHSVFrame();
	pRGB = m_pCamStream->getBGRFrame();
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

	//Find marker
	m_numCircle = 0;
	for (i=0; i<contours.size(); i++)
	{
		minEnclosingCircle(contours[i], center, radius);

		//New marker found
		if (contourArea(contours[i]) < m_areaRatio*radius*radius*3.1415926)continue;
		if (radius < m_minMarkerSize)continue;

		m_pCircle[m_numCircle].m_x = center.x;
		m_pCircle[m_numCircle].m_y = center.y;
		m_pCircle[m_numCircle].m_z = radius;
		m_numCircle++;

		if (m_numCircle == NUM_MARKER)
		{
			break;
		}
	}
}

void _MarkerDetector::detectCircleHough(void)
{
	if(!m_pCamStream)return;

	Frame* pFrame = m_pCamStream->getHSVFrame();
	if(pFrame->empty())return;
	if(!pFrame->isNewerThan(m_pFrame))return;
	m_pFrame->update(pFrame);

//	cv::Mat bgr_image = *m_pFrame->getCMat();
//	cv::medianBlur(bgr_image, bgr_image, m_kSize);

	// Convert input image to HSV
//	cv::Mat hsv_image;
//	cv::cvtColor(bgr_image, hsv_image, cv::COLOR_BGR2HSV);

	// Convert input image to HSV
	cv::Mat hsv_image = *m_pFrame->getCMat();
//	cv::medianBlur(hsv_image, hsv_image, 3);

	// Threshold the HSV image, keep only the red pixels
	cv::Mat lower_red_hue_range;
	cv::Mat upper_red_hue_range;
	cv::inRange(hsv_image, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), lower_red_hue_range);
	cv::inRange(hsv_image, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), upper_red_hue_range);

	// Combine the above two images
	cv::Mat red_hue_image;
	cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, red_hue_image);
	cv::GaussianBlur(red_hue_image, red_hue_image, cv::Size(9, 9), 2, 2);

	// Use the Hough transform to detect circles in the combined threshold image
	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(red_hue_image, circles, CV_HOUGH_GRADIENT, 1, m_houghMinDist, m_houghParam1, m_houghParam2, m_houghMinR, m_houghMaxR);


	m_numCircle = 0;
	if(circles.size() == 0)return;

	for(size_t current_circle = 0; current_circle < circles.size(); ++current_circle)
	{
		m_pCircle[m_numCircle].m_x = circles[current_circle][0];
		m_pCircle[m_numCircle].m_y = circles[current_circle][1];
		m_pCircle[m_numCircle].m_z = circles[current_circle][2];
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
	fVector3* pMarker = &m_pCircle[0];
	fVector3* pCompare;
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
	pCenter->m_z = pMarker->m_z;

	return true;
}


} /* namespace kai */
