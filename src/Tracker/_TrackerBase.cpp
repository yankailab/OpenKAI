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

bool _TrackerBase::updateROI(vDouble4& roi)
{
	NULL_F(m_pVision);
	Mat* pMat = m_pVision->BGR()->m();
	IF_F(pMat->empty());

	vInt4 iRoi;
	iRoi.x = roi.x * pMat->cols;
	iRoi.y = roi.y * pMat->rows;
	iRoi.z = roi.z * pMat->cols;
	iRoi.w = roi.w * pMat->rows;

	Rect2d rRoi;
	vInt42rect(iRoi,rRoi);
	IF_F(rRoi.width == 0 || rRoi.height == 0);

	m_ROI = rRoi;
	return true;
}

}
