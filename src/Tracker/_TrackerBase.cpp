/*
 * _TrackerBase.cpp
 *
 *  Created on: Aug 28, 2018
 *      Author: yankai
 */

#include "_TrackerBase.h"

namespace kai
{

_TrackerBase::_TrackerBase()
{
	m_pVision = NULL;
	m_trackerType = "";
	m_tStampBGR = 0;
	m_bTracking = false;
	m_bb.init();
}

_TrackerBase::~_TrackerBase()
{
}

bool _TrackerBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,trackerType);

	//link
	string iName = "";
	F_ERROR_F(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void _TrackerBase::createTracker(void)
{
}

void _TrackerBase::update(void)
{
}

bool _TrackerBase::bTracking(void)
{
	return m_bTracking;
}

vDouble4* _TrackerBase::getBB(void)
{
	return &m_bb;
}

bool _TrackerBase::updateBB(vDouble4& bb)
{
	NULL_F(m_pVision);
	Mat* pMat = m_pVision->BGR()->m();
	IF_F(pMat->empty());

	vInt4 iBB;
	iBB.x = bb.x * pMat->cols;
	iBB.y = bb.y * pMat->rows;
	iBB.z = bb.z * pMat->cols;
	iBB.w = bb.w * pMat->rows;

	Rect2d rBB;
	vInt42rect(iBB,rBB);
	IF_F(rBB.width == 0 || rBB.height == 0);

	m_rBB = rBB;
	m_bb = bb;
	return true;
}

}
