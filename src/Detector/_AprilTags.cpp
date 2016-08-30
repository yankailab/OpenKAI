/*
 * AprilTags.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_AprilTags.h"

namespace kai
{

_AprilTags::_AprilTags()
{
	_ThreadBase();
	DetectorBase();

	m_pCamStream = NULL;
	m_pFrame = NULL;
	m_tagFamily = "Tag16h5";
}

_AprilTags::~_AprilTags()
{
	// TODO Auto-generated destructor stub
}

bool _AprilTags::init(JSON* pJson, string name)
{
	CHECK_INFO(pJson->getVal("APRILTAGS_" + name + "_FAMILY", &m_tagFamily));

	double FPS = DEFAULT_FPS;
	CHECK_INFO(pJson->getVal("APRILTAGS_" + name + "_FPS", &FPS));
	this->setTargetFPS(FPS);

	m_pFrame = new Frame();

	return true;
}

bool _AprilTags::start(void)
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

void _AprilTags::update(void)
{
	TagFamily tagFamily(m_tagFamily);
	tagFamily.setErrorRecoveryFraction(0.1);
	m_pTagDetector = new TagDetector(m_tagFamily);

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}

}

void _AprilTags::detect(void)
{
	int i;

	if (!m_pCamStream)
		return;

	m_pFrame->update(m_pCamStream->getBGRFrame());
	if (m_pFrame->empty())
		return;

	TagDetectionArray detections;
	Mat* pImg = m_pFrame->getCMat();

//		while (std::max(src.rows, src.cols) > 800)
//		{
//			cv::Mat tmp;
//			cv::resize(src, tmp, cv::Size(0, 0), 0.5, 0.5);
//			src = tmp;
//		}

	cv::Point2d opticalCenter(0.5 * pImg->rows, 0.5 * pImg->cols);

	m_pTagDetector->process(*pImg, opticalCenter, detections);

	for (size_t i = 0; i < detections.size(); ++i)
	{
	}

	//Find marker
	m_numTag = 0;
	for (i = 0; i < detections.size(); i++)
	{
		TagDetection* pD = &detections[i];
		if (!pD->good)
			continue;

//		std::cout << " - Detection: id = " << d.id << ", " << "code = "
//				<< d.code << ", " << "rotation = " << d.rotation << "\n";

		m_pTag[m_numTag].m_x = pD->cxy.x;
		m_pTag[m_numTag].m_y = pD->cxy.y;
		m_numTag++;

		if (m_numTag == NUM_TAGS)
		{
			break;
		}
	}
}

bool _AprilTags::getTags(fVector3* pCenter)
{
	CamBase* pCam;

	if (pCenter == NULL)
		return false;
	if (m_pCamStream == NULL)
		return false;

//	pCam = m_pCamStream->getCameraInput();
//	if(pCam==NULL)return false;
//
//	//Use num instead of m_numCircle to avoid multi-thread inconsistancy
//	int num = m_numCircle;
//	if(num==0)return false;
//
//	int i;
//	fVector3* pMarker = &m_pCircle[0];
//	fVector3* pCompare;
//	int camCenter = (pCam->m_width+pCam->m_height)/2;
//
//	//Find the closest point
//	for(i=1; i<num; i++)
//	{
//		pCompare = &m_pCircle[i];
//		if(abs(pCompare->m_x + pCompare->m_y - camCenter) < abs(pMarker->m_x + pMarker->m_y - camCenter))
//		{
//			pMarker = pCompare;
//		}
//	}
//
//	pCenter->m_x = pMarker->m_x;
//	pCenter->m_y = pMarker->m_y;
//	pCenter->m_z = pMarker->m_z;

	return true;
}

bool _AprilTags::draw(Frame* pFrame, iVector4* pTextPos)
{
	int i;
	iVector3* pTag;

	if (pFrame == NULL)
		return false;

	Mat* pMat = pFrame->getCMat();

	for (i = 0; i < m_numTag; i++)
	{
		pTag = &m_pTag[i];

		circle(*pMat, Point(pTag->m_x, pTag->m_y), 10, Scalar(0, 0, 255), 5);
	}

	putText(*pMat, "AprilTags FPS: " + f2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

} /* namespace kai */
