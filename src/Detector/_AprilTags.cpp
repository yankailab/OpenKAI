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
	m_tagFamily = DEFAULT_TAG_FAMILY;
	m_tagErr = 0;
	m_numTags = NUM_TAGS;
	m_tagLifetime = USEC_1SEC;
	m_tagDistThr = 100;
	m_tagAliveInterval = USEC_1SEC;

	reset();
}

_AprilTags::~_AprilTags()
{
	// TODO Auto-generated destructor stub
}

bool _AprilTags::init(JSON* pJson, string name)
{
	CHECK_INFO(pJson->getVal("APRILTAGS_" + name + "_FAMILY", &m_tagFamily));
	CHECK_INFO(pJson->getVal("APRILTAGS_" + name + "_ERROR", &m_tagErr));
	CHECK_INFO(pJson->getVal("APRILTAGS_" + name + "_LIFETIME", &m_tagLifetime));
	CHECK_INFO(pJson->getVal("APRILTAGS_" + name + "_DIST_THR", &m_tagDistThr));
	CHECK_INFO(pJson->getVal("APRILTAGS_" + name + "_DET_INTERVAL", &m_tagAliveInterval));

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
	tagFamily.setErrorRecoveryFraction(m_tagErr);
	m_pTagDetector = new TagDetector(m_tagFamily);

	while (m_bThreadON)
	{
		this->autoFPSfrom();
		m_frameID = get_time_usec();
		m_tagAliveFrom = m_frameID - m_tagLifetime;

		detect();

		this->autoFPSto();
	}

}

void _AprilTags::detect(void)
{
	int i, j;

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

	for (i = 0; i < detections.size(); i++)
	{
		TagDetection* pD = &detections[i];

		if (!pD->good)
			continue;
		if (pD->id >= m_numTags)
			continue;

		for(j=0; j<NUM_PER_TAG;j++)
		{
			APRIL_TAG* pAT = &m_pTag[pD->id][j];

			if(abs(pAT->m_tag.cxy.x-pD->cxy.x)+abs(pAT->m_tag.cxy.y-pD->cxy.y) > m_tagDistThr && pAT->m_frameID>m_tagAliveFrom)continue;

			pAT->m_tag = *pD;
			pAT->m_detInterval = (m_frameID - pAT->m_frameID);
			pAT->m_frameID = m_frameID;
			break;
		}
	}

}

bool _AprilTags::getTags(int tagID, fVector3* pCenter)
{
	if (pCenter == NULL)
		return false;
	if (tagID > m_numTags)
		return false;

	APRIL_TAG* pAT = &m_pTag[tagID][0];
	if (pAT->m_frameID < m_tagAliveFrom)
		return false;

	pCenter->m_x = pAT->m_tag.cxy.x;
	pCenter->m_y = pAT->m_tag.cxy.y;

	return true;
}

void _AprilTags::reset(void)
{
	for (int i = 0; i < m_numTags; i++)
	{
		for(int j=0; j<NUM_PER_TAG; j++)
		{
			m_pTag[i][j].m_frameID = 0;
			m_pTag[i][j].m_detInterval = 0;
		}
	}
}

bool _AprilTags::draw(Frame* pFrame, iVector4* pTextPos)
{
	int i,j;
	APRIL_TAG* pTag;

	if (pFrame == NULL)
		return false;

	Mat* pMat = pFrame->getCMat();

	for (i = 0; i < m_numTags; i++)
	{
		for(j=0;j<NUM_PER_TAG;j++)
		{
			pTag = &m_pTag[i][j];
			if(pTag->m_frameID < m_tagAliveFrom)continue;
			if(pTag->m_detInterval > m_tagAliveInterval)continue;

			line(*pMat, pTag->m_tag.p[0], pTag->m_tag.p[1], Scalar(0, 0, 255), 1);
			line(*pMat, pTag->m_tag.p[1], pTag->m_tag.p[2], Scalar(0, 0, 255), 1);
			line(*pMat, pTag->m_tag.p[2], pTag->m_tag.p[3], Scalar(0, 0, 255), 1);
			line(*pMat, pTag->m_tag.p[3], pTag->m_tag.p[0], Scalar(0, 0, 255), 1);

			putText(*pMat, i2str(pTag->m_tag.id), pTag->m_tag.cxy,
					FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255), 1);
		}
	}

	putText(*pMat, "AprilTags FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

} /* namespace kai */
