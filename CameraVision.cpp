#include "CameraVision.h"

CameraVision::CameraVision()
{
}

CameraVision::~CameraVision()
{
}

bool CameraVision::init(void)
{
#if (SHOW_MARKER_FLOW == 1)
	namedWindow(MF_WINDOW);
#endif
#if (SHOW_OPTICAL_FLOW == 1)
	namedWindow(OPF_WINDOW);
#endif
#if (SHOW_FEATURE_FLOW == 1)
	namedWindow(CF_WINDOW);
#endif

	m_pMarkerFlow = NULL;
	m_pOpticalFlow = NULL;
	m_pFeatureFlow = NULL;

	m_Mat.m_frameID = 0;

	m_Mat.m_iFrame = 0;
	m_Mat.m_pOld = &m_Mat.m_pFrame[m_Mat.m_iFrame];
	m_Mat.m_pSmallOld = &m_Mat.m_pSmallFrame[m_Mat.m_iFrame];
	m_Mat.m_pGrayOld = &m_Mat.m_pGrayFrame[m_Mat.m_iFrame];
	m_Mat.m_pSmallGrayOld = &m_Mat.m_pSmallGrayFrame[m_Mat.m_iFrame];

	m_Mat.m_iFrame = 1 - m_Mat.m_iFrame;
	m_Mat.m_pNew = &m_Mat.m_pFrame[m_Mat.m_iFrame];
	m_Mat.m_pSmallNew = &m_Mat.m_pSmallFrame[m_Mat.m_iFrame];
	m_Mat.m_pGrayNew = &m_Mat.m_pGrayFrame[m_Mat.m_iFrame];
	m_Mat.m_pSmallGrayNew = &m_Mat.m_pSmallGrayFrame[m_Mat.m_iFrame];

	m_functionFlag = 0;
/*	m_functionFlag |= FRAME_SMALL;
	m_functionFlag |= FRAME_GRAY;
	m_functionFlag |= FRAME_GRAY_SMALL;
	m_functionFlag |= FRAME_HSV;
*/
	m_Mat.m_smallSize = cv::Size(SMALL_WIDTH,SMALL_HEIGHT);

	return true;
}

void CameraVision::initOpticalFlow(void)
{
	if (!m_pOpticalFlow)m_pOpticalFlow = new OPTICAL_FLOW();

	m_pOpticalFlow->m_flowMat = GpuMat(SMALL_WIDTH, SMALL_HEIGHT, CV_32FC2);
	m_pOpticalFlow->m_pFarn = cuda::FarnebackOpticalFlow::create();

	m_functionFlag |= FRAME_SMALL;
	m_functionFlag |= FRAME_GRAY_SMALL;

//	m_pOpticalFlow->m_method = OPF_FARN;
//	m_pOpticalFlow->m_pBrox = cuda::BroxOpticalFlow::create(0.197f, 50.0f, 0.8f, 10, 77, 10);
//	m_pOpticalFlow->m_pLK = cuda::DensePyrLKOpticalFlow::create(Size(7, 7));
//	m_pOpticalFlow->m_pTVL1 = cuda::OpticalFlowDual_TVL1::create();


}

void CameraVision::updateOpticalFlow(void)
{
	int i, j;
	cv::Point2f vFlow;
	double base;

	if (!m_pOpticalFlow)return;
	if (m_Mat.m_pSmallGrayOld->size() != m_Mat.m_pSmallGrayNew->size())return;
	if(m_pOpticalFlow->m_frameID == m_Mat.m_frameID)return;

	m_pOpticalFlow->m_pFarn->calc(*m_Mat.m_pSmallGrayOld, *m_Mat.m_pSmallGrayNew, m_pOpticalFlow->m_flowMat);
	m_pOpticalFlow->m_frameID = m_Mat.m_frameID;

	m_pOpticalFlow->m_flowMat.download(m_pOpticalFlow->m_uFlowMat);
	m_pOpticalFlow->m_flow = fVector4{ 0, 0, 0, 0 };

	for (i = 0; i < m_pOpticalFlow->m_uFlowMat.rows; i++)
	{
		for (j = 0; j < m_pOpticalFlow->m_uFlowMat.cols; j++)
		{
			vFlow = m_pOpticalFlow->m_uFlowMat.at<cv::Point2f>(i, j);
			m_pOpticalFlow->m_flow.m_x += vFlow.x;
			m_pOpticalFlow->m_flow.m_y += vFlow.y;
		}
	}

	base = 10.0/(double)(m_pOpticalFlow->m_uFlowMat.rows * m_pOpticalFlow->m_uFlowMat.cols);
	m_pOpticalFlow->m_flow.m_x *= base;
	m_pOpticalFlow->m_flow.m_y *= base;




/*	switch (m_pOpticalFlow->m_method)
	{
	case OPF_BROX:
		m_pOpticalFlow->m_pBrox->calc(*m_Mat.m_pSmallGrayOld, *m_Mat.m_pSmallGrayNew, m_pOpticalFlow->m_flowMat);
		break;
	case OPF_LK:
		m_pOpticalFlow->m_pLK->calc(*m_Mat.m_pSmallGrayOld, *m_Mat.m_pSmallGrayNew, m_pOpticalFlow->m_flowMat);
		break;
	case OPF_FARN:
		m_pOpticalFlow->m_pFarn->calc(*m_Mat.m_pSmallGrayOld, *m_Mat.m_pSmallGrayNew, m_pOpticalFlow->m_flowMat);
		break;
	case OPF_TVL1:
		m_pOpticalFlow->m_pTVL1->calc(*m_Mat.m_pSmallGrayOld, *m_Mat.m_pSmallGrayNew, m_pOpticalFlow->m_flowMat);
		break;
	default:
		m_pOpticalFlow->m_pBrox->calc(*m_Mat.m_pSmallGrayOld, *m_Mat.m_pSmallGrayNew, m_pOpticalFlow->m_flowMat);
		break;
	}
*/

#if (SHOW_OPTICAL_FLOW == 1)
	m_Mat.m_pNew->download(m_pOpticalFlow->m_uFlowMat);
	cv::Size mSize = m_pOpticalFlow->m_uFlowMat.size();
	Point p = Point(mSize.width / 2, mSize.height / 2);
	Point q = p + Point(m_pOpticalFlow->m_flow.m_x, m_pOpticalFlow->m_flow.m_y);
	line(m_pOpticalFlow->m_uFlowMat, p, q, Scalar(0, 255, 0), 3);
	imshow(OPF_WINDOW, m_pOpticalFlow->m_uFlowMat);
//	showFlow(OPF_WINDOW, m_pOpticalFlow->m_flowMat);
#endif

}

void CameraVision::getOpticalFlowVelocity(fVector4* pFlow)
{
	*pFlow = m_pOpticalFlow->m_flow;
}



void CameraVision::initFeatureFlow(void)
{
	if (!m_pFeatureFlow)m_pFeatureFlow = new FEATURE_FLOW();

	m_pFeatureFlow->m_flow = fVector4{ 0, 0, 0 };
	m_pFeatureFlow->m_diffPrev = fVector3{ 0, 0, 0 };

	m_functionFlag |= FRAME_GRAY;
	m_functionFlag |= FRAME_GRAY_SMALL;

#ifdef USE_CUDA
	m_pFeatureFlow->m_numCorners = 1000;
	m_pFeatureFlow->m_pDetector = cuda::createGoodFeaturesToTrackDetector(CV_8UC1, m_pFeatureFlow->m_numCorners, 0.01, 0.0);
	m_pFeatureFlow->m_pPyrLK = cuda::SparsePyrLKOpticalFlow::create();

	m_pFeatureFlow->m_pFrameNew = &m_Mat.m_pGrayNew;
	m_pFeatureFlow->m_pFrameOld = &m_Mat.m_pGrayOld;
#else
	//	m_pFeatureFlow->m_pDetector = FeatureDetector::create("AKAZE");
	//	m_pFeatureFlow->m_pExtractor = DescriptorExtractor::create("AKAZE");
	//	m_pFeatureFlow->m_pMatcher = DescriptorMatcher::create("BruteForce-Hamming");
#endif


}

void CameraVision::updateFeatureFlow(void)
{
#ifdef USE_CUDA
	GpuMat d_prevPts;
	GpuMat d_nextPts;
	GpuMat d_status;
	Mat   dMat, tMat;
	vector<Point2f> prevPts;
	vector<Point2f> nextPts;
	vector<uchar> status;
	cv::Size mSize;
	int i;
	int numPts;
	int numV;
	cv::Point p, q;
	double dX, dY;
	double distSQ, distSQTotal, absXtot, absYtot;
	double distSQMin;
	double distSQMax;
	double vBase;

	if (m_pFeatureFlow->m_frameID == m_Mat.m_frameID)return;
	m_pFeatureFlow->m_frameID = m_Mat.m_frameID;

	mSize = m_Mat.m_pGrayOld->size();
	if (mSize == Size(0, 0))return;
	if (mSize != m_Mat.m_pGrayNew->size())return;

	distSQMin = 1;
	distSQMax = (mSize.width + mSize.height)*0.15;
	distSQMax *= distSQMax;

	m_pFeatureFlow->m_pDetector->detect(**m_pFeatureFlow->m_pFrameOld, d_prevPts);
	m_pFeatureFlow->m_pPyrLK->calc(
		**m_pFeatureFlow->m_pFrameOld,
		**m_pFeatureFlow->m_pFrameNew,
		d_prevPts,
		d_nextPts,
		d_status);

	prevPts.resize(d_prevPts.cols);
	tMat = Mat(1, d_prevPts.cols, CV_32FC2, (void*)&prevPts[0]);
	d_prevPts.download(tMat);

	nextPts.resize(d_nextPts.cols);
	tMat = Mat(1, d_nextPts.cols, CV_32FC2, (void*)&nextPts[0]);
	d_nextPts.download(tMat);

	status.resize(d_status.cols);
	tMat = Mat(1, d_status.cols, CV_8UC1, (void*)&status[0]);
	d_status.download(tMat);

	//Find the flow direction
	m_pFeatureFlow->m_flow = fVector4{0, 0, 0, 0};
	absXtot = 0.0;
	absYtot = 0.0;
	numPts = prevPts.size();
	numV = 0;

	for ( i = 0; i < numPts; i++)
	{
		if (!status[i])continue;

		p = prevPts[i];
		q = nextPts[i];

		dX = q.x - p.x;
		dY = q.y - p.y;
		distSQ = dX*dX + dY*dY;

		if (distSQ < distSQMin)continue;
		if (distSQ > distSQMax)continue;

		m_pFeatureFlow->m_flow.m_x += dX;
		m_pFeatureFlow->m_flow.m_y += dY;
		absXtot += abs(dX);
		absYtot += abs(dY);
		numV++;
	}

	//Calc the motion vector
	vBase = 1.0/(double)numV;
	distSQTotal = absXtot*absXtot + absYtot*absYtot;

	m_pFeatureFlow->m_flow.m_x *= vBase;
	m_pFeatureFlow->m_flow.m_y *= vBase;
	m_pFeatureFlow->m_flow.m_z = 0;	//TODO: Change in Height
	m_pFeatureFlow->m_flow.m_w = 0;	//TODO: Rotation

#if (SHOW_FEATURE_FLOW==1)	
	m_Mat.m_pNew->download(dMat);
	p = Point(mSize.width / 2, mSize.height / 2);
	q = p + Point(m_pFeatureFlow->m_flow.m_x, m_pFeatureFlow->m_flow.m_y);
	line(dMat, p, q, Scalar(0,255,0), 3);
//	drawFeatureFlow(dMat, prevPts, nextPts, status, Scalar(255, 0, 0));
	imshow(CF_WINDOW, dMat);
#endif

#else

	m_pFeatureFlow->m_pDetector->detect(m_Mat.m_uFrame, m_pFeatureFlow->m_keypoint1);
	m_pFeatureFlow->m_pDetector->detect(m_Mat.m_uFrame, m_pFeatureFlow->m_keypoint2);

	m_pFeatureFlow->m_pExtractor->compute(m_Mat.m_uFrame, m_pFeatureFlow->m_keypoint1, m_pFeatureFlow->m_descriptor1);
	m_pFeatureFlow->m_pExtractor->compute(m_Mat.m_uFrame, m_pFeatureFlow->m_keypoint2, m_pFeatureFlow->m_descriptor2);

	m_pFeatureFlow->m_pMatcher->match(m_pFeatureFlow->m_descriptor1, m_pFeatureFlow->m_descriptor2, m_pFeatureFlow->m_dMatch);	
#endif
}

void CameraVision::getFeatureFlowVelocity(fVector4* pFlow)
{
	*pFlow = m_pFeatureFlow->m_flow;
}

void CameraVision::initMarkerFlow(void)
{
	if (!m_pMarkerFlow)m_pMarkerFlow = new MARKER_FLOW();

	m_pMarkerFlow->m_numAllMarker = 0;
	m_pMarkerFlow->m_objLockLevel = LOCK_LEVEL_NONE;
	m_pMarkerFlow->m_objPos.m_z = 0;

	m_functionFlag |= FRAME_HSV;
//	m_functionFlag |= FRAME_SMALL;


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
	m_pMarkerFlow->m_pBlobDetector = SimpleBlobDetector::create(params);

	// SimpleBlobDetector::create creates a smart pointer. 
	// So you need to use arrow ( ->) instead of dot ( . )
	// detector->detect( im, keypoints);

	*/
}



void CameraVision::updateMarkerFlow(void)
{
	int i,j,k;
	Point2f center;
	float radius;
	vector< vector< Point > > contours;
	vector<Vec3f> circles;
	fVector4 v4tmp;
	UMat matThresh;

	if (m_pMarkerFlow->m_frameID == m_Mat.m_frameID)return;
	m_pMarkerFlow->m_frameID = m_Mat.m_frameID;

	//Filtering the image
	cuda::absdiff(m_Mat.m_Hue, Scalar(90), m_pMarkerFlow->m_Huered);
	cuda::multiply(m_pMarkerFlow->m_Huered, Scalar(0.25), m_pMarkerFlow->m_Scalehuered);	//1/4
	cuda::multiply(m_Mat.m_Sat, Scalar(0.0625), m_pMarkerFlow->m_Scalesat);	//1/16
	cuda::multiply(m_pMarkerFlow->m_Scalehuered, m_pMarkerFlow->m_Scalesat, m_pMarkerFlow->m_Balloonyness);
	cuda::threshold(m_pMarkerFlow->m_Balloonyness, m_pMarkerFlow->m_Thresh, 200, 255, THRESH_BINARY);
//	cuda::threshold(m_pMarkerFlow->m_Balloonyness, m_pMarkerFlow->m_Thresh, 200, 255, THRESH_BINARY_INV);
	m_pMarkerFlow->m_Thresh.download(matThresh);


#if (SHOW_MARKER_FLOW==1)	
//	Mat matShow;
//	m_pMarkerFlow->m_Thresh.download(matShow);
//	m_Mat.m_pSmallNew->download(matShow);

	// Storage for blobs
//	vector<KeyPoint> keypoints;
	// Detect blobs
//	m_pMarkerFlow->m_pBlobDetector->detect(matShow, keypoints);

	// Draw detected blobs as red circles.
	// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures
	// the size of the circle corresponds to the size of blob

//	Mat imTest;
//	drawKeypoints(matShow, keypoints, imTest, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

//	imshow(MF_WINDOW, matThresh);

//	m_pMarkerFlow->m_objLockLevel = LOCK_LEVEL_NONE;
//	return;
#endif

/*	m_Balloonyness.download(m_threshMat);
	/// Reduce the noise so we avoid false circle detection
	GaussianBlur(m_threshMat, m_testMat, Size(9, 9), 2, 2);
	/// Apply the Hough Transform to find the circles
	HoughCircles(m_testMat, circles, CV_HOUGH_GRADIENT, 1, 100, 200, 100, 0, 0);

	/// Draw the circles detected
	m_pMarkerFlow->m_numAllMarker = 0;
	for (size_t i = 0; i < circles.size(); i++)
	{
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		// circle outline
		circle(m_Mat.m_uFrame, center, radius, Scalar(0, 255, 0), 3);

		m_pMarkerFlow->m_pAllMarker[m_pMarkerFlow->m_numAllMarker].m_x = center.x;
		m_pMarkerFlow->m_pAllMarker[m_pMarkerFlow->m_numAllMarker].m_y = center.y;
		m_pMarkerFlow->m_pAllMarker[m_pMarkerFlow->m_numAllMarker].m_z = radius;
		m_pMarkerFlow->m_pAllMarker[m_pMarkerFlow->m_numAllMarker].m_w = abs(center.x - m_pMarkerFlow->m_objROIPos.m_x) + abs(center.y - m_pMarkerFlow->m_objROIPos.m_y);
		m_pMarkerFlow->m_numAllMarker++;

		if (m_pMarkerFlow->m_numAllMarker == NUM_MARKER)
		{
			break;
		}
	}
*/
	
	//Find the contours
	findContours(matThresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

#if (DRAW_DEBUG_DATA == 1)
	drawContours(m_Mat.m_uFrame, contours, -1, Scalar(255, 0, 0));
#endif
	
	//Find marker
	m_pMarkerFlow->m_numAllMarker = 0;
	for (i=0; i<contours.size(); i++)
	{
		minEnclosingCircle(contours[i], center, radius);

#if (DRAW_DEBUG_DATA == 1)
//		circle(m_Mat.m_uFrame, center, radius, Scalar(0, 255, 255));
#endif

		//New marker found
		if (contourArea(contours[i]) < m_pMarkerFlow->m_areaRatio*radius*radius*3.1415926)continue;
		if (radius < m_pMarkerFlow->m_minMarkerSize)continue;

		circle(m_Mat.m_uFrame, center, radius, Scalar(0, 255, 0), 2);

		m_pMarkerFlow->m_pAllMarker[m_pMarkerFlow->m_numAllMarker].m_x = center.x;
		m_pMarkerFlow->m_pAllMarker[m_pMarkerFlow->m_numAllMarker].m_y = center.y;
		m_pMarkerFlow->m_pAllMarker[m_pMarkerFlow->m_numAllMarker].m_z = radius;
		m_pMarkerFlow->m_pAllMarker[m_pMarkerFlow->m_numAllMarker].m_w = abs(center.x - m_pMarkerFlow->m_objROIPos.m_x) + abs(center.y - m_pMarkerFlow->m_objROIPos.m_y);
		m_pMarkerFlow->m_numAllMarker++;

		if (m_pMarkerFlow->m_numAllMarker == NUM_MARKER)
		{
			break;
		}
	}


	//Failed to detect any marker
	if (m_pMarkerFlow->m_numAllMarker <= 0)
	{
		m_pMarkerFlow->m_objLockLevel = LOCK_LEVEL_NONE;
		return;
	}

	//Sorting the markers from near to far
	for (i=1; i<m_pMarkerFlow->m_numAllMarker; i++)
	{
		for (j=i-1; j>=0; j--)
		{
			if (m_pMarkerFlow->m_pAllMarker[j+1].m_w > m_pMarkerFlow->m_pAllMarker[j].m_w)
			{
				break;
			}

			v4tmp = m_pMarkerFlow->m_pAllMarker[j];
			m_pMarkerFlow->m_pAllMarker[j] = m_pMarkerFlow->m_pAllMarker[j+1];
			m_pMarkerFlow->m_pAllMarker[j+1] = v4tmp;
		}
	}

	for (i = 0; i < NUM_TARGET_MARKER; i++)
	{
		m_pMarkerFlow->m_pTargetMarker[i] = m_pMarkerFlow->m_pAllMarker[i];
	}

	//if detected markers are less than expected, only use the first one
	if (m_pMarkerFlow->m_numAllMarker < NUM_TARGET_MARKER)
	{
		m_pMarkerFlow->m_objPos.m_x = m_pMarkerFlow->m_pAllMarker[0].m_x;
		m_pMarkerFlow->m_objPos.m_y = m_pMarkerFlow->m_pAllMarker[0].m_y;
		m_pMarkerFlow->m_objROIPos = m_pMarkerFlow->m_objPos;

		//only position is locked
		m_pMarkerFlow->m_objLockLevel = LOCK_LEVEL_POS;
		return;
	}
	
	//Determine the center position and size of object
	m_pMarkerFlow->m_objPos.m_x = (m_pMarkerFlow->m_pAllMarker[1].m_x + m_pMarkerFlow->m_pAllMarker[0].m_x)*0.5;
	m_pMarkerFlow->m_objPos.m_y = (m_pMarkerFlow->m_pAllMarker[1].m_y + m_pMarkerFlow->m_pAllMarker[0].m_y)*0.5;
	m_pMarkerFlow->m_objPos.m_z = abs(m_pMarkerFlow->m_pAllMarker[1].m_x - m_pMarkerFlow->m_pAllMarker[0].m_x);
	m_pMarkerFlow->m_objROIPos = m_pMarkerFlow->m_objPos;
	m_pMarkerFlow->m_objLockLevel = LOCK_LEVEL_SIZE;

	//Determine the attitude of object
	m_pMarkerFlow->m_objAtt.m_x = (m_pMarkerFlow->m_pAllMarker[1].m_y - m_pMarkerFlow->m_pAllMarker[0].m_y) / (m_pMarkerFlow->m_pAllMarker[1].m_x - m_pMarkerFlow->m_pAllMarker[0].m_x);
	m_pMarkerFlow->m_objAtt.m_y = 0; //TODO: how to determine pitch?

	if (m_pMarkerFlow->m_pAllMarker[0].m_x < m_pMarkerFlow->m_pAllMarker[1].m_x)
	{
		m_pMarkerFlow->m_objAtt.m_z = m_pMarkerFlow->m_pAllMarker[1].m_z / m_pMarkerFlow->m_pAllMarker[0].m_z;
	}
	else
	{
		m_pMarkerFlow->m_objAtt.m_z = m_pMarkerFlow->m_pAllMarker[0].m_z / m_pMarkerFlow->m_pAllMarker[1].m_z;
	}


	m_pMarkerFlow->m_objLockLevel = LOCK_LEVEL_ATT;

}

int CameraVision::getObjLockLevel(void)
{
	return 	m_pMarkerFlow->m_objLockLevel;
}

bool CameraVision::getObjPosition(fVector3* pPos)
{
	if (m_pMarkerFlow == NULL)return false;
	if (m_pMarkerFlow->m_objLockLevel < LOCK_LEVEL_POS)return false;
	pPos->m_x = m_pMarkerFlow->m_objPos.m_x;
	pPos->m_y = m_pMarkerFlow->m_objPos.m_y;
	if (m_pMarkerFlow->m_objLockLevel >= LOCK_LEVEL_SIZE)
	{
		pPos->m_z = m_pMarkerFlow->m_objPos.m_z;
	}
	return true;
}

bool CameraVision::getObjAttitude(fVector3* pAtt)
{
	if (m_pMarkerFlow == NULL)return false;
	if (m_pMarkerFlow->m_objLockLevel<LOCK_LEVEL_ATT)return false;
	*pAtt = m_pMarkerFlow->m_objAtt;
	return true;
}

void CameraVision::setObjROI(fVector3 ROI)
{
	m_pMarkerFlow->m_objROIPos = ROI;
}


void CameraVision::updateFrame(UMat* pFrame)
{
	if (pFrame == NULL)return;

	//Update the frame ID for flow synchronization
	if (++m_Mat.m_frameID == MAX_FRAME_ID)
	{
		m_Mat.m_frameID = 0;
	}

	m_Mat.m_uFrame = *pFrame;

	//switch the current frame and old frame
	m_Mat.m_pOld = m_Mat.m_pNew;
	m_Mat.m_pSmallOld = m_Mat.m_pSmallNew;
	m_Mat.m_pGrayOld = m_Mat.m_pGrayNew;
	m_Mat.m_pSmallGrayOld = m_Mat.m_pSmallGrayNew;

	m_Mat.m_iFrame = 1 - m_Mat.m_iFrame;

	m_Mat.m_pNew = &m_Mat.m_pFrame[m_Mat.m_iFrame];
	m_Mat.m_pSmallNew = &m_Mat.m_pSmallFrame[m_Mat.m_iFrame];
	m_Mat.m_pGrayNew = &m_Mat.m_pGrayFrame[m_Mat.m_iFrame];
	m_Mat.m_pSmallGrayNew = &m_Mat.m_pSmallGrayFrame[m_Mat.m_iFrame];

	//Generating Gray and Small frames if needed
#ifdef USE_CUDA
	UMat   uMatTmp;
	GpuMat hsv;
	vector<GpuMat> hsvPlanes(3);

	m_Mat.m_pNew->upload(m_Mat.m_uFrame);

	if (m_functionFlag & FRAME_SMALL)
	{
		cv::resize(m_Mat.m_uFrame, uMatTmp, m_Mat.m_smallSize);
		m_Mat.m_pSmallNew->upload(uMatTmp);
	}

	if (m_functionFlag & FRAME_GRAY)
	{
		cuda::cvtColor(*m_Mat.m_pNew, *m_Mat.m_pGrayNew, CV_BGR2GRAY);
	}

	if (m_functionFlag & FRAME_GRAY_SMALL)
	{
		cuda::cvtColor(*m_Mat.m_pSmallNew, *m_Mat.m_pSmallGrayNew, CV_BGR2GRAY);
	}

	if (m_functionFlag & FRAME_HSV)
	{
		hsvPlanes[0] = m_Mat.m_Hue;
		hsvPlanes[1] = m_Mat.m_Sat;
		hsvPlanes[2] = m_Mat.m_Val;

		//RGB or BGR depends on device
		cuda::cvtColor(*m_Mat.m_pNew, hsv, CV_BGR2HSV);
		cuda::split(hsv, hsvPlanes);

		m_Mat.m_Hue = hsvPlanes[0];
		m_Mat.m_Sat = hsvPlanes[1];
		m_Mat.m_Val = hsvPlanes[2];
	}


#else


#endif



}

void CameraVision::getGlobalFlowVelocity(fVector4* pFlow)
{
	if (!pFlow)return;

	int		 numMethod = 0;
	double	 nBase;
	fVector4 vFlow = fVector4{ 0, 0, 0, 0 };

	if (m_pMarkerFlow)
	{
		if (m_pMarkerFlow->m_frameID == m_Mat.m_frameID)
		{
			vFlow.m_x += m_pMarkerFlow->m_flow.m_x;
			vFlow.m_y += m_pMarkerFlow->m_flow.m_y;
			vFlow.m_z += m_pMarkerFlow->m_flow.m_z;
			vFlow.m_w += m_pMarkerFlow->m_flow.m_w;

			numMethod++;
		}
	}

	if (m_pFeatureFlow)
	{
		if (m_pFeatureFlow->m_frameID == m_Mat.m_frameID)
		{
			vFlow.m_x += m_pFeatureFlow->m_flow.m_x;
			vFlow.m_y += m_pFeatureFlow->m_flow.m_y;
			vFlow.m_z += m_pFeatureFlow->m_flow.m_z;
			vFlow.m_w += m_pFeatureFlow->m_flow.m_w;

			numMethod++;
		}
	}

	if (m_pOpticalFlow)
	{
		if (m_pOpticalFlow->m_frameID == m_Mat.m_frameID)
		{
			vFlow.m_x += m_pOpticalFlow->m_flow.m_x;
			vFlow.m_y += m_pOpticalFlow->m_flow.m_y;
			vFlow.m_z += m_pOpticalFlow->m_flow.m_z;
			vFlow.m_w += m_pOpticalFlow->m_flow.m_w;

			numMethod++;
		}
	}

	if (numMethod == 0)
	{
		*pFlow = fVector4{ 0, 0, 0, 0 };
		return;
	}

	nBase = 1.0 / (double)numMethod;
	pFlow->m_x = vFlow.m_x*nBase;
	pFlow->m_y = vFlow.m_y*nBase;
	pFlow->m_z = vFlow.m_z*nBase;
	pFlow->m_w = vFlow.m_w*nBase;

}


cv::UMat* CameraVision::getMat(void)
{

#if (SHOW_DEBUG_WINDOW ==1)
/*	Mat hue_host, sat_host, val_host, balloonyness_host;
	m_pMarkerFlow->m_Hue.download(hue_host);
	m_Mat.m_Sat.download(sat_host);
	m_Mat.m_Val.download(val_host);
	m_Balloonyness.download(balloonyness_host);
	imshow("hue", hue_host);
	imshow("sat", sat_host);
	imshow("val", val_host);
	imshow("balloonyness", balloonyness_host);
	*/
#endif

	return &m_Mat.m_uFrame;
}

inline bool CameraVision::isFlowCorrect(Point2f u)
{
	return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
}

Vec3b CameraVision::computeColor(float fx, float fy)
{
	static bool first = true;

	// relative lengths of color transitions:
	// these are chosen based on perceptual similarity
	// (e.g. one can distinguish more shades between red and yellow
	//  than between yellow and green)
	const int RY = 15;
	const int YG = 6;
	const int GC = 4;
	const int CB = 11;
	const int BM = 13;
	const int MR = 6;
	const int NCOLS = RY + YG + GC + CB + BM + MR;
	static Vec3i colorWheel[NCOLS];

	if (first)
	{
		int k = 0;

		for (int i = 0; i < RY; ++i, ++k)
			colorWheel[k] = Vec3i(255, 255 * i / RY, 0);

		for (int i = 0; i < YG; ++i, ++k)
			colorWheel[k] = Vec3i(255 - 255 * i / YG, 255, 0);

		for (int i = 0; i < GC; ++i, ++k)
			colorWheel[k] = Vec3i(0, 255, 255 * i / GC);

		for (int i = 0; i < CB; ++i, ++k)
			colorWheel[k] = Vec3i(0, 255 - 255 * i / CB, 255);

		for (int i = 0; i < BM; ++i, ++k)
			colorWheel[k] = Vec3i(255 * i / BM, 0, 255);

		for (int i = 0; i < MR; ++i, ++k)
			colorWheel[k] = Vec3i(255, 0, 255 - 255 * i / MR);

		first = false;
	}

	const float rad = sqrt(fx * fx + fy * fy);
	const float a = atan2(-fy, -fx) / (float)CV_PI;

	const float fk = (a + 1.0f) / 2.0f * (NCOLS - 1);
	const int k0 = static_cast<int>(fk);
	const int k1 = (k0 + 1) % NCOLS;
	const float f = fk - k0;

	Vec3b pix;

	for (int b = 0; b < 3; b++)
	{
		const float col0 = colorWheel[k0][b] / 255.0f;
		const float col1 = colorWheel[k1][b] / 255.0f;

		float col = (1 - f) * col0 + f * col1;

		if (rad <= 1)
			col = 1 - rad * (1 - col); // increase saturation with radius
		else
			col *= .75; // out of range

		pix[2 - b] = static_cast<uchar>(255.0 * col);
	}

	return pix;
}

void CameraVision::drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion)
{
	dst.create(flowx.size(), CV_8UC3);
	dst.setTo(Scalar::all(0));

	// determine motion range:
	float maxrad = maxmotion;

	if (maxmotion <= 0)
	{
		maxrad = 1;
		for (int y = 0; y < flowx.rows; ++y)
		{
			for (int x = 0; x < flowx.cols; ++x)
			{
				Point2f u(flowx(y, x), flowy(y, x));

				if (!isFlowCorrect(u))
					continue;

				maxrad = max(maxrad, sqrt(u.x * u.x + u.y * u.y));
			}
		}
	}

	for (int y = 0; y < flowx.rows; ++y)
	{
		for (int x = 0; x < flowx.cols; ++x)
		{
			Point2f u(flowx(y, x), flowy(y, x));

			if (isFlowCorrect(u))
				dst.at<Vec3b>(y, x) = computeColor(u.x / maxrad, u.y / maxrad);
		}
	}
}

void CameraVision::showFlow(const char* name, const GpuMat& d_flow)
{
	GpuMat planes[2];
	cuda::split(d_flow, planes);

	Mat flowx(planes[0]);
	Mat flowy(planes[1]);

	Mat out;
	drawOpticalFlow(flowx, flowy, out, 10);

	imshow(name, out);
}



void CameraVision::drawFeatureFlow(Mat& frame, const vector<Point2f>& prevPts, const vector<Point2f>& nextPts, const vector<uchar>& status, Scalar line_color)
{
	for (size_t i = 0; i < prevPts.size(); ++i)
	{
		if (status[i])
		{
			int line_thickness = 1;

			Point p = prevPts[i];
			Point q = nextPts[i];

			double angle = atan2((double)p.y - q.y, (double)p.x - q.x);

			double hypotenuse = sqrt((double)(p.y - q.y)*(p.y - q.y) + (double)(p.x - q.x)*(p.x - q.x));

			if (hypotenuse < 1.0)
				continue;
			if (hypotenuse > 100.0)
				continue;

			// Here we lengthen the arrow by a factor of three.
			q.x = (int)(p.x - 3 * hypotenuse * cos(angle));
			q.y = (int)(p.y - 3 * hypotenuse * sin(angle));

			// Now we draw the main line of the arrow.
			line(frame, p, q, line_color, line_thickness);

			// Now draw the tips of the arrow. I do some scaling so that the
			// tips look proportional to the main line of the arrow.

			p.x = (int)(q.x + 9 * cos(angle + CV_PI / 4));
			p.y = (int)(q.y + 9 * sin(angle + CV_PI / 4));
			line(frame, p, q, line_color, line_thickness);

			p.x = (int)(q.x + 9 * cos(angle - CV_PI / 4));
			p.y = (int)(q.y + 9 * sin(angle - CV_PI / 4));
			line(frame, p, q, line_color, line_thickness);
		}
	}
}




void CameraVision::recordTime(long delta, double *avgTime)
{
	*avgTime = (*avgTime * m_pBenchmark->m_nFrames + delta) / (m_pBenchmark->m_nFrames + 1);
}

void CameraVision::log(const char* msg, ...)
{
#if (PER_FRAME_TIME_LOGGING == 1)
	va_list args;
	va_start(args, msg);
	printf(msg, args);
#endif
}



