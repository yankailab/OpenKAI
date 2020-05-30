/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_Universe.h"

namespace kai
{

_Universe::_Universe()
{
	m_minConfidence = 0.0;
	m_minArea = -1.0;
	m_maxArea = -1.0;
	m_minW = -1.0;
	m_maxW = -1.0;
	m_minH = -1.0;
	m_maxH = -1.0;
	m_bbScale = -1.0;
	m_bMerge = false;
	m_mergeOverlap = 0.8;
	m_vRoi.init(0.0, 0.0, 1.0, 1.0);

	m_bDrawStatistics = false;
	m_bDrawClass = false;

	clearObj();
}

_Universe::~_Universe()
{
}

bool _Universe::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//general
	pK->v("minConfidence", &m_minConfidence);
	pK->v("minArea", &m_minArea);
	pK->v("maxArea", &m_maxArea);
	pK->v("minW", &m_minW);
	pK->v("minH", &m_minH);
	pK->v("maxW", &m_maxW);
	pK->v("maxH", &m_maxH);
	pK->v("bbScale", &m_bbScale);

	//draw
	pK->v("bDrawStatistics", &m_bDrawStatistics);
	pK->v("bDrawClass", &m_bDrawClass);

	m_pO[0].init(pKiss);
	m_pO[1].init(pKiss);
	clearObj();

	return true;
}

void _Universe::clearObj(void)
{
	m_iSwitch = 0;
	updateObj();
	m_pPrev->clear();
	m_pNext->clear();
}

void _Universe::updateObj(void)
{
	m_iSwitch = 1 - m_iSwitch;
	m_pPrev = &m_pO[m_iSwitch];
	m_pNext = &m_pO[1 - m_iSwitch];
	m_pNext->clear();
}

bool _Universe::start(void)
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

void _Universe::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->autoFPSto();
	}
}

_Object* _Universe::add(_Object& o)
{
	float area = o.area();
	IF_N(m_minArea >= 0 && area < m_minArea);
	IF_N(m_maxArea >= 0 && area > m_maxArea);
	IF_N(m_minW >= 0 && o.getW() < m_minW);
	IF_N(m_maxW >= 0 && o.getW() > m_maxW);
	IF_N(m_minH >= 0 && o.getH() < m_minW);
	IF_N(m_maxH >= 0 && o.getH() > m_maxH);

	vFloat3 p = o.getPos();
	IF_N(p.x < m_vRoi.x);
	IF_N(p.x > m_vRoi.z);
	IF_N(p.y < m_vRoi.y);
	IF_N(p.y > m_vRoi.w);

//	if(m_bbScale > 0.0)
//	{
//		o.setBB2D(bbScale(o.getBB2D(), m_bbScale));
//	}

//	if(m_bMerge)
//	{
//		vFloat4 BB = o.getBB2D();
//
//		for(int i=0; i<m_pNext->size(); i++)
//		{
//			OBJECT* pO = &m_pNext->get(i);
//			if(!pO)break;
//			IF_CONT(pO->m_topClass != o.m_topClass);
//			IF_CONT(nIoU(BB, pO->m_bb) < m_mergeOverlap);
//
//			pO->m_bb.x = small(pNewO->m_bb.x, pO->m_bb.x);
//			pO->m_bb.y = small(pNewO->m_bb.y, pO->m_bb.y);
//			pO->m_bb.z = big(pNewO->m_bb.z, pO->m_bb.z);
//			pO->m_bb.w = big(pNewO->m_bb.w, pO->m_bb.w);
//			pO->m_topProb = big(pNewO->m_topProb, pO->m_topProb);
//			pO->m_mClass |= pNewO->m_mClass;
//
//			return pO;
//		}
//	}

	return m_pNext->add(o);
}

_Object* _Universe::get(int i)
{
	return m_pPrev->get(i);
}

void _Universe::draw(void)
{
	this->_ThreadBase::draw();
}

}
