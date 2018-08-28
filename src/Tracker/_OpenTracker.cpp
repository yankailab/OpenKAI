/*
 * _OpenTracker.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_OpenTracker.h"

#ifdef USE_OPENTRACKER

namespace kai
{

_OpenTracker::_OpenTracker()
{
}

_OpenTracker::~_OpenTracker()
{
}

bool _OpenTracker::init(void* pKiss)
{
	IF_F(!this->_TrackerBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool _OpenTracker::link(void)
{
	IF_F(!this->_TrackerBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _OpenTracker::start(void)
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

void _OpenTracker::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		track();

		this->autoFPSto();
	}
}

bool _OpenTracker::updateROI(vDouble4& roi)
{
	IF_F(!this->_TrackerBase::updateROI(roi));

	createTracker();
	Mat* pMat = m_pVision->BGR()->m();
	Rect2f r = m_ROI;
	m_eco.init(*pMat, r);
	m_bTracking = true;

	return true;
}

void _OpenTracker::track(void)
{
	NULL_(m_pVision);
	IF_(!m_bTracking);

	Frame* pFrame = m_pVision->BGR();
	IF_(pFrame->tStamp() <= m_tStampBGR);
	m_tStampBGR = pFrame->tStamp();

	Mat* pMat = pFrame->m();
	IF_(pMat->empty());

	Rect2f r = m_ROI;
	m_eco.update(*pMat, r);
}

}
#endif
