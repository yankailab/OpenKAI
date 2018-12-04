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
	m_pDet = NULL;
	m_trackerType = "";
	m_tStampBGR = 0;
	m_trackState = track_stop;
	m_bb.init();
	m_iSet = 0;
	m_iInit = 0;
	m_margin = 0.0;

}

_TrackerBase::~_TrackerBase()
{
}

bool _TrackerBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,trackerType);
	KISSm(pK,margin);

	//link
	string iName = "";
	F_ERROR_F(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInst(iName));

	iName = "";
	F_ERROR_F(pK->v("_ObjectBase", &iName));
	m_pDet = (_ObjectBase*) (pK->root()->getChildInst(iName));

	return true;
}

void _TrackerBase::createTracker(void)
{
}

void _TrackerBase::update(void)
{
}

void _TrackerBase::stopTrack(void)
{
	m_trackState = track_stop;
}

TRACK_STATE _TrackerBase::trackState(void)
{
	return m_trackState;
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

	double mBig = 1.0 + m_margin;
	double mSmall = 1.0 - m_margin;

	bb.x = constrain(bb.x * mSmall, 0.0, 1.0);
	bb.y = constrain(bb.y * mSmall, 0.0, 1.0);
	bb.z = constrain(bb.z * mBig, 0.0, 1.0);
	bb.w = constrain(bb.w * mBig, 0.0, 1.0);

	vInt4 iBB;
	iBB.x = bb.x * pMat->cols;
	iBB.y = bb.y * pMat->rows;
	iBB.z = bb.z * pMat->cols;
	iBB.w = bb.w * pMat->rows;

	Rect2d rBB;
	vInt42rect(iBB,rBB);
	IF_F(rBB.width == 0 || rBB.height == 0);

	m_newBB = rBB;

	m_iSet++;
	m_trackState = track_init;
	return true;
}

bool _TrackerBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	Scalar col;
	string msg = "Stop";

	if(m_trackState == track_init)
	{
		col = Scalar(0,255,255);
		rectangle(*pMat, m_newBB, col, 2);
		msg = "Init";
	}
	else if(m_trackState == track_update)
	{
		col = Scalar(0,255,0);
		rectangle(*pMat, m_rBB, col, 2);
		msg = "Update";
	}

	pWin->tabNext();
	pWin->addMsg(&msg);
	pWin->tabPrev();

	return true;
}

bool _TrackerBase::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg = "Stop";
	if(m_trackState == track_init)
	{
		msg = "Init";
	}
	else if(m_trackState == track_update)
	{
		msg = "Update";
	}
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Tracking pos = ("
			+ f2str(m_bb.midX()) + ", "
			+ f2str(m_bb.midY()) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
