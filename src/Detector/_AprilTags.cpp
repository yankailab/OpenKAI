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

	m_pStream = NULL;
	m_pFrame = NULL;
	m_tagFamily = DEFAULT_TAG_FAMILY;
	m_tagErr = 0;
	m_numTags = NUM_TAGS;
	m_tagLifetime = USEC_1SEC;
	m_tagDistThr = 100;
	m_tagAliveInterval = USEC_1SEC;
	m_tagScaling = 0.8;
	m_tagSizeLim = 800;

	reset();
}

_AprilTags::~_AprilTags()
{
	// TODO Auto-generated destructor stub
}

bool _AprilTags::init(Kiss* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	pKiss->m_pInst = this;

	//format params
	F_INFO(pKiss->v("family", &m_tagFamily));
	F_INFO(pKiss->v("err", &m_tagErr));
	F_INFO(pKiss->v("lifeTime", &m_tagLifetime));
	F_INFO(pKiss->v("distThr", &m_tagDistThr));
	F_INFO(pKiss->v("detInterval", &m_tagAliveInterval));
	F_INFO(pKiss->v("scaling", &m_tagScaling));
	F_INFO(pKiss->v("sizeLim", &m_tagSizeLim));

	m_pFrame = new Frame();

	return true;
}

bool _AprilTags::link(void)
{
	NULL_F(m_pKiss);

	//link instance
	string iName = "";
	F_ERROR_F(m_pKiss->v("_Stream",&iName));
	m_pStream = (_Stream*)(m_pKiss->root()->getChildInstByName(&iName));

	//TODO: link variables to Automaton

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

	if (!m_pStream)
		return;

	m_pFrame->update(m_pStream->getBGRFrame());
	if (m_pFrame->empty())
		return;

	TagDetectionArray detections;
	Mat* pImg = m_pFrame->getCMat();
	double scaling = 1.0;

	while (std::max(pImg->rows, pImg->cols) > m_tagSizeLim)
	{
		scaling *= m_tagScaling;
		cv::Mat tmp;
		cv::resize(*pImg, tmp, cv::Size(0, 0), m_tagScaling, m_tagScaling);
		*pImg = tmp;
	}
	cv::Point2d opticalCenter(0.5 * pImg->rows, 0.5 * pImg->cols);

	m_pTagDetector->process(*pImg, opticalCenter, detections);

	scaling = 1.0/scaling;

	for (i = 0; i < detections.size(); i++)
	{
		TagDetection* pD = &detections[i];

		if (!pD->good)
			continue;
		if (pD->id >= m_numTags)
			continue;

		if(abs(scaling-1.0) > 0.0000001)
		{
			pD->cxy.x *= scaling;
			pD->cxy.y *= scaling;
			pD->p[0] *= scaling;
			pD->p[1] *= scaling;
			pD->p[2] *= scaling;
			pD->p[3] *= scaling;
		}

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

int _AprilTags::getTags(int tagID, APRIL_TAG* pTag)
{
	if (pTag == NULL)
		return 0;
	if (tagID > m_numTags)
		return 0;

	int i,k;
	k=0;
	for(i=0; i<NUM_PER_TAG;i++)
	{
		APRIL_TAG* pAT = &m_pTag[tagID][i];
		if (pAT->m_frameID < m_tagAliveFrom)continue;

		pTag[k]=*pAT;
		k++;
	}

	return k;
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

bool _AprilTags::draw(Frame* pFrame, vInt4* pTextPos)
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
