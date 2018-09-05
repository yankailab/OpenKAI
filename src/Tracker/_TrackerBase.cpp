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

bool _TrackerBase::startTrack(vDouble4& bb)
{
	NULL_F(m_pVision);
	Mat* pMat = m_pVision->BGR()->m();
	IF_F(pMat->empty());

	bb.x = constrain(bb.x,0.0,1.0);
	bb.y = constrain(bb.y,0.0,1.0);
	bb.z = constrain(bb.z,0.0,1.0);
	bb.w = constrain(bb.w,0.0,1.0);

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

void _TrackerBase::stopTrack(void)
{
	m_bTracking = false;
}

bool _TrackerBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	rectangle(*pMat, m_rBB, Scalar(255,255,0), 1);

	return true;
}

bool _TrackerBase::cli(int& iY)
{
	IF_F(!this->_ThreadBase::cli(iY));

	string msg = "Tracking pos = ("
				+ f2str(m_bb.midX()) + ", "
				+ f2str(m_bb.midY()) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
