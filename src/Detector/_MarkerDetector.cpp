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

	m_bThreadON = false;
	m_threadID = 0;
	m_cudaDeviceID = 0;

	m_numAllMarker = 0;
	m_objLockLevel = LOCK_LEVEL_NONE;
	m_objPos.m_z = 0;

	m_minMarkerSize = MIN_MARKER_SIZE;
	m_areaRatio = MARKER_AREA_RATIO;

	m_tSleep = TRD_INTERVAL_MARKERDETECTOR;
	m_pCamStream = NULL;
}

_MarkerDetector::~_MarkerDetector()
{
	// TODO Auto-generated destructor stub
}


bool _MarkerDetector::init(JSON* pJson, string name)
{

	CHECK_INFO(pJson->getVal("MARKER_DETECTOR_TSLEEP_" + name, &m_tSleep));

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
		this->updateTime();

		detect();

		//sleepThread can be woke up by this->wakeupThread()
		this->sleepThread(0, m_tSleep);
	}

}

void _MarkerDetector::detect(void)
{
	int i,j,k;
	Point2f center;
	float radius;
	vector< vector< Point > > contours;
	vector<Vec3f> circles;
	fVector4 v4tmp;
	UMat matThresh;
	CamFrame* pHSV;
	CamFrame* pRGB;

	if(!m_pCamStream)return;
	pHSV = m_pCamStream->m_pHSV;
	pRGB = m_pCamStream->m_pFrameL;
	if(pRGB->empty())return;
	if(pHSV->empty())return;

	vector<GpuMat> matHSV(3);
	matHSV[0] = m_Hue;
	matHSV[1] = m_Sat;
	matHSV[2] = m_Val;

	cuda::split(*(pHSV->getCurrentFrame()), matHSV);

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
	m_numAllMarker = 0;
	for (i=0; i<contours.size(); i++)
	{
		minEnclosingCircle(contours[i], center, radius);

//			circle(pRGB->m_uFrame, center, radius, Scalar(0, 255, 0), 1);

		//New marker found
		if (contourArea(contours[i]) < m_areaRatio*radius*radius*3.1415926)continue;
		if (radius < m_minMarkerSize)continue;

//			circle(pRGB->m_uFrame, center, radius, Scalar(0, 255, 0), 2);

		m_pAllMarker[m_numAllMarker].m_x = center.x;
		m_pAllMarker[m_numAllMarker].m_y = center.y;
		m_pAllMarker[m_numAllMarker].m_z = radius;
		m_pAllMarker[m_numAllMarker].m_w = abs(center.x - m_objROIPos.m_x) + abs(center.y - m_objROIPos.m_y);
		m_numAllMarker++;

		if (m_numAllMarker == NUM_MARKER)
		{
			break;
		}
	}

	//Failed to detect any marker
	if (m_numAllMarker <= 0)
	{
		m_objLockLevel = LOCK_LEVEL_NONE;
		return;
	}

	//Sorting the markers from near to far
	for (i=1; i<m_numAllMarker; i++)
	{
		for (j=i-1; j>=0; j--)
		{
			if (m_pAllMarker[j+1].m_w > m_pAllMarker[j].m_w)
			{
				break;
			}

			v4tmp = m_pAllMarker[j];
			m_pAllMarker[j] = m_pAllMarker[j+1];
			m_pAllMarker[j+1] = v4tmp;
		}
	}

	for (i = 0; i < NUM_TARGET_MARKER; i++)
	{
		m_pTargetMarker[i] = m_pAllMarker[i];
	}

	//if detected markers are less than expected, only use the first one
	if (m_numAllMarker < NUM_TARGET_MARKER)
	{
		m_objPos.m_x = m_pAllMarker[0].m_x;
		m_objPos.m_y = m_pAllMarker[0].m_y;
		m_objPos.m_z = m_pAllMarker[0].m_z;
		m_objROIPos = m_objPos;

		//only position is locked
		m_objLockLevel = LOCK_LEVEL_POS;
		return;
	}

	//Determine the center position and size of object
	m_objPos.m_x = (m_pAllMarker[1].m_x + m_pAllMarker[0].m_x)*0.5;
	m_objPos.m_y = (m_pAllMarker[1].m_y + m_pAllMarker[0].m_y)*0.5;
	m_objPos.m_z = abs(m_pAllMarker[1].m_x - m_pAllMarker[0].m_x);
	m_objROIPos = m_objPos;
	m_objLockLevel = LOCK_LEVEL_SIZE;

	//Determine the attitude of object
	m_objAtt.m_x = (m_pAllMarker[1].m_y - m_pAllMarker[0].m_y) / (m_pAllMarker[1].m_x - m_pAllMarker[0].m_x);
	m_objAtt.m_y = 0; //TODO: how to determine pitch?

	if (m_pAllMarker[0].m_x < m_pAllMarker[1].m_x)
	{
		m_objAtt.m_z = m_pAllMarker[1].m_z / m_pAllMarker[0].m_z;
	}
	else
	{
		m_objAtt.m_z = m_pAllMarker[0].m_z / m_pAllMarker[1].m_z;
	}

	m_objLockLevel = LOCK_LEVEL_ATT;

}

int _MarkerDetector::getObjLockLevel(void)
{
	return 	m_objLockLevel;
}

bool _MarkerDetector::getObjPosition(fVector3* pPos)
{
	if (m_objLockLevel < LOCK_LEVEL_POS)return false;
	pPos->m_x = m_objPos.m_x;
	pPos->m_y = m_objPos.m_y;
//	if (m_objLockLevel >= LOCK_LEVEL_SIZE)
//	{
		pPos->m_z = m_objPos.m_z;
//	}
	return true;
}

bool _MarkerDetector::getObjAttitude(fVector3* pAtt)
{
	if (m_objLockLevel<LOCK_LEVEL_ATT)return false;
	*pAtt = m_objAtt;
	return true;
}

void _MarkerDetector::setObjROI(fVector3 ROI)
{
	m_objROIPos = ROI;
}


/*
	// Setup SimpleBlobDetector parameters.
	SimpleBlobDetector::Params params;

	// Change thresholds
	params.minThreshold = 0;
	params.maxThreshold = 100;

	// Filter by Area.
	params.filterByArea = false;
//	params.minArea = 10;
//	params.maxArea = 10;

	// Filter by Circularity
	params.filterByCircularity = true;
	params.minCircularity = 0.8;

	// Filter by Convexity
	params.filterByConvexity = true;
	params.minConvexity = 0.9;

	// Filter by Inertia
	params.filterByInertia = true;
	params.minInertiaRatio = 0.2;


	// Set up detector with params
	m_pBlobDetector = SimpleBlobDetector::create(params);

	// SimpleBlobDetector::create creates a smart pointer.
	// So you need to use arrow ( ->) instead of dot ( . )
	// detector->detect( im, keypoints);
*/
/*	m_Balloonyness.download(m_threshMat);
	/// Reduce the noise so we avoid false circle detection
	GaussianBlur(m_threshMat, m_testMat, Size(9, 9), 2, 2);
	/// Apply the Hough Transform to find the circles
	HoughCircles(m_testMat, circles, CV_HOUGH_GRADIENT, 1, 100, 200, 100, 0, 0);

	/// Draw the circles detected
	m_numAllMarker = 0;
	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		// circle outline
		circle(m_Mat.m_uFrame, center, radius, Scalar(0, 255, 0), 3);

		m_pAllMarker[m_numAllMarker].m_x = center.x;
		m_pAllMarker[m_numAllMarker].m_y = center.y;
		m_pAllMarker[m_numAllMarker].m_z = radius;
		m_pAllMarker[m_numAllMarker].m_w = abs(center.x - m_objROIPos.m_x) + abs(center.y - m_objROIPos.m_y);
		m_numAllMarker++;

		if (m_numAllMarker == NUM_MARKER)
		{
			break;
		}
	}
*/


} /* namespace kai */
