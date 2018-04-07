/*
 * _VisionBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_VisionBase.h"

namespace kai
{

_VisionBase::_VisionBase()
{
	m_bOpen = false;
	m_type = unknownStream;
	m_w = 1280;
	m_h = 720;
	m_cW = 640;
	m_cH = 360;
	m_bGimbal = false;
	m_isoScale = 1.0;
	m_rotTime = 0;
	m_rotPrev = 0;
	m_fovW = 60;
	m_fovH = 60;
	m_bFlip = false;

	m_pBGR = NULL;
	m_pHSV = NULL;
	m_pGray = NULL;
}

_VisionBase::~_VisionBase()
{
	reset();
}

bool _VisionBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,w);
	KISSm(pK,h);
	KISSm(pK,fovW);
	KISSm(pK,fovH);
	KISSm(pK,bGimbal);
	KISSm(pK,isoScale);
	KISSm(pK,bFlip);

	m_pBGR = new Frame();

	bool bParam = false;
	F_INFO(pK->v("bGray", &bParam));
	if (bParam)
		m_pGray = new Frame();

	bParam = false;
	F_INFO(pK->v("bHSV", &bParam));
	if (bParam)
		m_pHSV = new Frame();

	m_bOpen = false;
	return true;
}

void _VisionBase::reset(void)
{
	this->_ThreadBase::reset();

	DEL(m_pBGR);
	DEL(m_pHSV);
	DEL(m_pGray);
}

Frame* _VisionBase::bgr(void)
{
	return m_pBGR;
}

Frame* _VisionBase::hsv(void)
{
	return m_pHSV;
}

Frame* _VisionBase::gray(void)
{
	return m_pGray;
}

void _VisionBase::info(vInt2* pSize, vInt2* pCenter, vInt2* pAngle)
{
	if(pSize)
	{
		pSize->x = m_w;
		pSize->y = m_h;
	}

	if(pCenter)
	{
		pCenter->x = m_cW;
		pCenter->y = m_cH;
	}

	if(pAngle)
	{
		pAngle->x = m_fovW;
		pAngle->y = m_fovH;
	}
}

void _VisionBase::setAttitude(double rollRad, double pitchRad, uint64_t timestamp)
{
	Point2f center(m_cW, m_cH);
	double deg = -rollRad * 180.0 * OneOvPI;

	m_rotRoll = getRotationMatrix2D(center, deg, m_isoScale);
	//TODO: add rot estimation

}

Mat* _VisionBase::K(void)
{
	return &m_K;
}

VISION_TYPE _VisionBase::getType(void)
{
	return m_type;
}

bool _VisionBase::isOpened(void)
{
	return m_bOpen;
}

bool _VisionBase::draw(void)
{
	NULL_F(m_pWindow);
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();

	IF_F(m_pBGR->empty());
	pFrame->update(m_pBGR);

	return this->_ThreadBase::draw();
}

}
