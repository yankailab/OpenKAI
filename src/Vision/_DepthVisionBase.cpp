/*
 * _DepthVisionBase.cpp
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#include "_DepthVisionBase.h"

namespace kai
{

_DepthVisionBase::_DepthVisionBase()
{
	m_pDepthWin = NULL;
	m_wD = 1280;
	m_hD = 720;
}

_DepthVisionBase::~_DepthVisionBase()
{
}

bool _DepthVisionBase::init(void* pKiss)
{
	IF_F(!this->_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,wD);
	KISSm(pK,hD);

	//link
	string iName = "";
	F_INFO(pK->v("depthWindow", &iName));
	m_pDepthWin = (Window*) (pK->root()->getChildInst(iName));

	return true;
}

double _DepthVisionBase::d(vDouble4* pROI, vInt2* pPos)
{
	IF__(!pROI, -1.0);
	IF__(m_fDepth.bEmpty(),-1.0);

	Size s = m_fDepth.size();

	vInt4 iR;
	iR.x = pROI->x * s.width;
	iR.y = pROI->y * s.height;
	iR.z = pROI->z * s.width;
	iR.w = pROI->w * s.height;

	if (iR.x < 0)
		iR.x = 0;
	if (iR.y < 0)
		iR.y = 0;
	if (iR.z > s.width)
		iR.z = s.width;
	if (iR.w > s.height)
		iR.w = s.height;

	return d(&iR, pPos);
}

double _DepthVisionBase::d(vInt4* pROI, vInt2* pPos)
{
	IF__(!pROI, -1.0);
	IF__(m_fDepth.bEmpty(),-1.0);

	Rect roi;
	vInt42rect(*pROI, roi);
	double d = cv::sum((*m_fDepth.m())(roi))[0];

	return d/(roi.width*roi.height);
}

Frame* _DepthVisionBase::Depth(void)
{
	return &m_fDepth;
}

bool _DepthVisionBase::draw(void)
{
	IF_F(!this->_VisionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Frame* pFrame = pWin->getFrame();

	IF_F(m_depthShow.bEmpty());
	IF_F(!m_pDepthWin);

	m_pDepthWin->getFrame()->copy(*m_depthShow.m());

	return true;
}

}
