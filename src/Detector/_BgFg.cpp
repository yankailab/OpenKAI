
#include "_BgFg.h"

namespace kai
{

_BgFg::_BgFg()
{
	_ThreadBase();
	DetectorBase();

	m_cudaDeviceID = 0;

	m_objPos.m_z = 0;

	m_minSize = MIN_MARKER_SIZE;
	m_objLockLevel = LOCK_LEVEL_NONE;

	m_pCamStream = NULL;
}

_BgFg::~_BgFg()
{
	// TODO Auto-generated destructor stub
}


bool _BgFg::init(JSON* pJson)
{
//	CHECK_INFO(pJson->getVal("BGFG_DETECTOR_TSLEEP_" + name, &m_tSleep));

    m_pBgSubtractor = cuda::createBackgroundSubtractorMOG();
//    cuda::createBackgroundSubtractorMOG2();
//    cuda::createBackgroundSubtractorGMG(40);
//    cuda::createBackgroundSubtractorFGD();

    this->setTargetFPS(30.0);

	return true;
}

bool _BgFg::start(void)
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

void _BgFg::update(void)
{
	cuda::setDevice(m_cudaDeviceID);

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}

}

void _BgFg::detect(void)
{
	int i;
	Point2f center;
	float radius;
	vector< vector< Point > > contours;
	vector<Vec3f> circles;
	UMat matThresh;
	Frame* pRGB;

	if(!m_pCamStream)return;
	pRGB = m_pCamStream->getBGRFrame();//m_pCamStream->m_pFrameL;
	if(pRGB->empty())return;

	m_pBgSubtractor->apply(*pRGB->getGMat(), m_gFg);
//	m_pBgSubtractor->getBackgroundImage(m_gBg);

	pRGB->getGMat()->download(m_Mat);
	m_gFg.download(matThresh);

	//Find the contours
	findContours(matThresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
//	drawContours(m_Mat, contours, -1, Scalar(0, 255, 0), 2);

	//Find marker
	m_numAllMarker = 0;
	for (i=0; i<contours.size(); i++)
	{
		minEnclosingCircle(contours[i], center, radius);
//		circle(m_Mat, center, radius, Scalar(0, 255, 0), 1);

		//New marker found
		if (contourArea(contours[i]) < 10)continue;
		if (radius < m_minSize)continue;

		circle(m_Mat, center, radius, Scalar(0, 255, 0), 2);

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

	//TODO: use the center point of all points as the vehicle position

	//Sorting the markers from near to far
//	for (i=1; i<m_numAllMarker; i++)
//	{
//		for (j=i-1; j>=0; j--)
//		{
//			if (m_pAllMarker[j+1].m_w > m_pAllMarker[j].m_w)
//			{
//				break;
//			}
//
//			v4tmp = m_pAllMarker[j];
//			m_pAllMarker[j] = m_pAllMarker[j+1];
//			m_pAllMarker[j+1] = v4tmp;
//		}
//	}


		m_objPos.m_x = m_pAllMarker[0].m_x;
		m_objPos.m_y = m_pAllMarker[0].m_y;
		m_objPos.m_z = m_pAllMarker[0].m_z;
		m_objROIPos = m_objPos;

		//only position is locked
		m_objLockLevel = LOCK_LEVEL_POS;

	//Determine the center position and size of object
/*	m_objPos.m_x = (m_pAllMarker[1].m_x + m_pAllMarker[0].m_x)*0.5;
	m_objPos.m_y = (m_pAllMarker[1].m_y + m_pAllMarker[0].m_y)*0.5;
	m_objPos.m_z = abs(m_pAllMarker[1].m_x - m_pAllMarker[0].m_x);
	m_objROIPos = m_objPos;
	m_objLockLevel = LOCK_LEVEL_SIZE;

	m_objLockLevel = LOCK_LEVEL_ATT;
*/
}


bool _BgFg::getObjPosition(vDouble3* pPos)
{
	if (m_objLockLevel < LOCK_LEVEL_POS)return false;
	pPos->m_x = m_objPos.m_x;
	pPos->m_y = m_objPos.m_y;
	pPos->m_z = m_objPos.m_z;

	return true;
}

void _BgFg::setObjROI(vDouble3 ROI)
{
	m_objROIPos = ROI;
}



} /* namespace kai */
