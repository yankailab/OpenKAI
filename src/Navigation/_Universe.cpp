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

	m_nObjClass = 0;
	m_nObj = NUM_OBJ;
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

bool _Universe::init(void* pKiss)
{
	CHECK_F(this->_ThreadBase::init(pKiss) == false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_ERROR_F(pK->v("frameLifetime", &m_frameLifeTime));
	F_ERROR_F(pK->v("probMin", &m_objProbMin));
	F_ERROR_F(pK->v("posDisparity", &m_disparity));

	return true;
}

bool _Universe::link(void)
{
	NULL_F(m_pKiss);

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
	pObj->m_iClass = 0;
}

void _Universe::reset(void)
{
	for (int i = 0; i < m_nObj; i++)
	{
		deleteObject(i);
	}
}

void _Universe::updateObject(void)
{
	OBJECT* pObj;

	//Collect the candidates
	for (int i = 0; i < m_nObj; i++)
	{
		pObj = &m_pObj[i];

		if (pObj->m_frameID <= 0)continue;

		//Delete the outdated frame
		if (m_frameID - pObj->m_frameID > m_frameLifeTime)
		{
			deleteObject(i);
			continue;
		}
	}
}

int _Universe::addObjClass(string* pName, uint8_t safety)
{
	if(pName==NULL)return -1;
	if(m_nObjClass >= NUM_OBJ_CLASS)return -1;

	m_pObjClass[m_nObjClass].m_name = *pName;
	m_pObjClass[m_nObjClass].m_safety = safety;
	m_nObjClass++;
	return m_nObjClass-1;
}

OBJECT* _Universe::addObject(OBJECT* pNewObj)
{
	NULL_N(pNewObj);

	int i;
	int iVacant;
	OBJECT* pObj;
	uint64_t frameID = get_time_usec();

	iVacant = m_nObj;
	for (i = 0; i < m_nObj; i++)
	{
		pObj = &m_pObj[i];

		if (pObj->m_frameID > 0)	//>0:not vacant
		{
			//compare if already existed
			if (abs(pObj->m_bbox.m_x - pNewObj->m_bbox.m_x) > m_disparity)
				continue;
			if (abs(pObj->m_bbox.m_y - pNewObj->m_bbox.m_y) > m_disparity)
				continue;
			if (abs(pObj->m_bbox.m_z - pNewObj->m_bbox.m_z) > m_disparity)
				continue;
			if (abs(pObj->m_bbox.m_w - pNewObj->m_bbox.m_w) > m_disparity)
				continue;

			//already existed, update and return
			*pObj = *pNewObj;
			pObj->m_frameID = frameID;
			return pObj;
		}

		//found an index of vacancy
		iVacant = i;
		break;
	}

	CHECK_N(iVacant >= m_nObj);

	//Change in status comes to the last
	pObj = &m_pObj[iVacant];
	*pObj = *pNewObj;
	pObj->m_frameID = frameID;
	return pObj;

}

OBJECT* _Universe::getObjectByClass(int iClass)
{
	int i;
	OBJECT* pObj;

	for (i = 0; i < m_nObj; i++)
	{
		pObj = &m_pObj[i];

		if (pObj->m_frameID <= 0)	//>0:not vacant
			continue;

		if (pObj->m_iClass == iClass)
			return pObj;
	}

	return NULL;
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

	for (int i = 0; i < m_nObj; i++)
	{
		pObj = &m_pObj[i];
		if (pObj->m_frameID <= 0)continue;

		color = Scalar(0, 255, 0);
		thickness = 1;
		Rect bbox;
		bbox.x = pObj->m_bbox.m_x;
		bbox.y = pObj->m_bbox.m_y;
		bbox.width = pObj->m_bbox.m_z - pObj->m_bbox.m_x;
		bbox.height = pObj->m_bbox.m_w - pObj->m_bbox.m_y;

		rectangle(*pMat, bbox, color, thickness);

		if(pObj->m_iClass >= m_nObjClass)continue;
		putText(*pMat, m_pObjClass[pObj->m_iClass].m_name,
				Point(bbox.x + bbox.width / 2, bbox.y + bbox.height / 2),
				FONT_HERSHEY_SIMPLEX, 0.8, color, thickness);
	}

	return true;

}

} /* namespace kai */
