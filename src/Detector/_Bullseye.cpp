/*
 * CameraMarkerDetect.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_Bullseye.h"

namespace kai
{

_Bullseye::_Bullseye()
{
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

	m_pStream = NULL;
	m_pFrame = NULL;
}

_Bullseye::~_Bullseye()
{
}

bool _Bullseye::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	//format params
	F_ERROR_F(pK->v("areaRatio", &m_areaRatio));
	F_ERROR_F(pK->v("minSize", &m_minMarkerSize));

	F_INFO(pK->v("method", &m_method));
	F_INFO(pK->v("medBlueKsize", &m_kSize));

	F_INFO(pK->v("HoughMinDist", &m_houghMinDist));
	F_INFO(pK->v("HoughParam1", &m_houghParam1));
	F_INFO(pK->v("HoughParam2", &m_houghParam2));
	F_INFO(pK->v("HoughMinR", &m_houghMinR));
	F_INFO(pK->v("HoughMaxR", &m_houghMaxR));

	m_pFrame = new Frame();

	return true;
}

bool _Bullseye::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	//link instance
	string iName = "";
	F_ERROR_F(pK->v("_Stream",&iName));
	m_pStream = (_VisionBase*)(pK->root()->getChildInstByName(&iName));

	return true;
}

bool _Bullseye::start(void)
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

void _Bullseye::update(void)
{
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


void _Bullseye::detectCircleFill(void)
{
	int i;
	Point2f center;
	float radius;
	vector< vector< Point > > contours;
	vector<Vec3f> circles;
	Mat matThresh;
	Frame* pHSV;
	Frame* pRGB;

	if(!m_pStream)return;
	pHSV = m_pStream->hsv();
	pRGB = m_pStream->bgr();
	if(pRGB->empty())return;
	if(pHSV->empty())return;

	vector<GpuMat> matHSV(3);
	matHSV[0] = m_Hue;
	matHSV[1] = m_Sat;
	matHSV[2] = m_Val;

	split(*(pHSV->getGMat()), matHSV);

	m_Hue = matHSV[0];
	m_Sat = matHSV[1];
	m_Val = matHSV[2];

	//Filtering the image
#ifdef USE_OPENCV2X
	gpu::absdiff(m_Hue, Scalar(90), m_Huered);
	gpu::multiply(m_Huered, Scalar(0.25), m_Scalehuered);	//1/4
	gpu::multiply(m_Sat, Scalar(0.0625), m_Scalesat);	//1/16
	gpu::multiply(m_Scalehuered, m_Scalesat, m_Balloonyness);
	gpu::threshold(m_Balloonyness, m_Thresh, 200, 255, THRESH_BINARY);
//	gpu::threshold(m_Balloonyness, m_Thresh, 200, 255, THRESH_BINARY_INV);
#else
	cuda::absdiff(m_Hue, Scalar(90), m_Huered);
	cuda::multiply(m_Huered, Scalar(0.25), m_Scalehuered);	//1/4
	cuda::multiply(m_Sat, Scalar(0.0625), m_Scalesat);	//1/16
	cuda::multiply(m_Scalehuered, m_Scalesat, m_Balloonyness);
	cuda::threshold(m_Balloonyness, m_Thresh, 200, 255, THRESH_BINARY);
//	cuda::threshold(m_Balloonyness, m_Thresh, 200, 255, THRESH_BINARY_INV);
#endif

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

		m_pCircle[m_numCircle].x = center.x;
		m_pCircle[m_numCircle].y = center.y;
		m_pCircle[m_numCircle].z = radius;
		m_numCircle++;

		if (m_numCircle == NUM_MARKER)
		{
			break;
		}
	}
}

void _Bullseye::detectCircleHough(void)
{
	if(!m_pStream)return;

	Frame* pFrame = m_pStream->hsv();
	NULL_(pFrame);
	IF_(pFrame->empty());
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
		m_pCircle[m_numCircle].x = circles[current_circle][0];
		m_pCircle[m_numCircle].y = circles[current_circle][1];
		m_pCircle[m_numCircle].z = circles[current_circle][2];
		m_numCircle++;

		if (m_numCircle == NUM_MARKER)
		{
			break;
		}

	}
}

bool _Bullseye::getCircleCenter(vDouble3* pCenter)
{
	NULL_F(pCenter);
	NULL_F(m_pStream);

	//Use num instead of m_numCircle to avoid multi-thread inconsistancy
	int num = m_numCircle;
	if(num==0)return false;

	int i;
	vDouble3* pMarker = &m_pCircle[0];
	vDouble3* pCompare;
	int camCenter = (m_pStream->m_w+m_pStream->m_h)/2;

	//Find the closest point
	for(i=1; i<num; i++)
	{
		pCompare = &m_pCircle[i];
		if(abs(pCompare->x + pCompare->y - camCenter) < abs(pMarker->x + pMarker->y - camCenter))
		{
			pMarker = pCompare;
		}
	}

	pCenter->x = pMarker->x;
	pCenter->y = pMarker->y;
	pCenter->z = pMarker->z;

	return true;
}


} /* namespace kai */
