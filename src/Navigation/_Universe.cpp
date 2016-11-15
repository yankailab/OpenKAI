/*
 * _Universe.cpp
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#include "_Universe.h"

namespace kai
{

_Universe::_Universe()
{
	_ThreadBase();

	m_numObj = NUM_OBJ;
	m_frameID = 0;
	m_frameLifeTime = 0;
	m_objProbMin = 0;
	m_disparity = 0;
	reset();

}

_Universe::~_Universe()
{
	// TODO Auto-generated destructor stub
}

bool _Universe::init(Config* pConfig)
{
	CHECK_F(this->_ThreadBase::init(pConfig) == false);
	pConfig->m_pInst = this;

	F_ERROR_F(pConfig->v("frameLifetime", &m_frameLifeTime));
	F_ERROR_F(pConfig->v("probMin", &m_objProbMin));
	F_ERROR_F(pConfig->v("posDisparity", &m_disparity));

	return true;
}

bool _Universe::link(void)
{
	NULL_F(m_pConfig);

	return true;
}

bool _Universe::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Universe::update(void)
{

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_frameID = get_time_usec();

		updateObject();

		this->autoFPSto();
	}

}

void _Universe::deleteObject(int i)
{
	OBJECT* pObj = &m_pObj[i];

	pObj->m_frameID = 0;
	pObj->m_prob = 0;
}

void _Universe::updateObject(void)
{
	OBJECT* pObj;

	//Collect the candidates
	for (int i = 0; i < m_numObj; i++)
	{
		pObj = &m_pObj[i];

		//Delete the outdated frame
		if (m_frameID - pObj->m_frameID > m_frameLifeTime)
		{
			deleteObject(i);
			continue;
		}
	}
}

OBJECT* _Universe::addObject(OBJ_CLASS* pClass, vInt4* pBbox, double dist,
		double prob)
{
	NULL_N(pBbox);

	int i;
	int iVacant;
	OBJECT* pObj;
	uint64_t frameID = get_time_usec();

	iVacant = m_numObj;
	for (i = 0; i < m_numObj; i++)
	{
		pObj = &m_pObj[i];

		if (pObj->m_frameID)	//>0:not vacant
		{
			//compare if already existed
			if (abs(pObj->m_bbox.m_x - pBbox->m_x) > m_disparity)
				continue;
			if (abs(pObj->m_bbox.m_y - pBbox->m_y) > m_disparity)
				continue;
			if (abs(pObj->m_bbox.m_z - pBbox->m_z) > m_disparity)
				continue;
			if (abs(pObj->m_bbox.m_w - pBbox->m_w) > m_disparity)
				continue;

			//already existed, update and return
			pObj->m_frameID = frameID;
			pObj->m_bbox = *pBbox;
			pObj->m_dist = dist;
			if (!pClass)
				return pObj;
			if (prob > pObj->m_prob)
			{
				pObj->m_pClass = pClass;
				pObj->m_prob = prob;
			}

			return pObj;
		}

		//found an index of vacancy
		iVacant = i;
		break;
	}

	CHECK_N(iVacant >= m_numObj);

	//Change in status comes to the last
	pObj = &m_pObj[iVacant];
	pObj->m_frameID = frameID;
	pObj->m_pClass = pClass;
	pObj->m_prob = prob;
	pObj->m_bbox = *pBbox;
	pObj->m_dist = dist;
	return pObj;

}

void _Universe::reset(void)
{
	for (int i = 0; i < m_numObj; i++)
	{
		m_pObj[i].m_frameID = 0;
		m_pObj[i].m_prob = 0;
	}
}

bool _Universe::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);

	Mat* pMat = pFrame->getCMat();
	putText(*pMat, "Universe FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);
	pTextPos->m_y += pTextPos->m_w;

	OBJECT* pObj;
	Scalar color;
	double thickness;

	for (int i = 0; i < m_numObj; i++)
	{
		pObj = &m_pObj[i];
		if (pObj->m_frameID <= 0)
			continue;

		color = Scalar(0, 255, 0);
		thickness = 1;
		Rect bbox;
		bbox.x = pObj->m_bbox.m_x;
		bbox.y = pObj->m_bbox.m_y;
		bbox.width = pObj->m_bbox.m_z - pObj->m_bbox.m_x;
		bbox.height = pObj->m_bbox.m_w - pObj->m_bbox.m_y;

		rectangle(*pMat, bbox, color, thickness);

		if(!pObj->m_pClass)continue;
		putText(*pMat, pObj->m_pClass->m_name,
				Point(bbox.x + bbox.width / 2,
					  bbox.y + bbox.height / 2),
				FONT_HERSHEY_SIMPLEX, 0.8, color, thickness);
	}

	return true;

}

} /* namespace kai */
