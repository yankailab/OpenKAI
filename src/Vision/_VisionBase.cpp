/*
 * _VisionBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_VisionBase.h"

#ifdef USE_OPENCV

namespace kai
{

_VisionBase::_VisionBase()
{
	m_bOpen = false;
	m_type = vision_unknown;
    m_vSize.init(1280,720);
    m_vFov.init(60,60);
	m_bbDraw.x = -1.0;
}

_VisionBase::~_VisionBase()
{
}

bool _VisionBase::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("vSize",&m_vSize);
    pK->v("vFov",&m_vFov);
	pK->v("bbDraw",&m_bbDraw);

	m_bOpen = false;
	return true;
}

Frame* _VisionBase::BGR(void)
{
	return &m_fBGR;
}

vInt2 _VisionBase::getSize(void)
{
	return m_vSize;
}

VISION_TYPE _VisionBase::getType(void)
{
	return m_type;
}

bool _VisionBase::open(void)
{
	return true;
}

bool _VisionBase::isOpened(void)
{
	return m_bOpen;
}

void _VisionBase::close(void)
{
    IF_(check()<0);

    m_pT->goSleep();
	while(!m_pT->bSleeping());

	m_bOpen = false;
}

void _VisionBase::draw(void)
{
	NULL_(m_pWindow);
	Frame* pFrame = ((_WindowCV*)m_pWindow)->getFrame();

	if(!m_fBGR.bEmpty())
	{
		if(m_bbDraw.x < 0.0)
		{
			pFrame->copy(m_fBGR);
		}
		else
		{
			Mat* pM = pFrame->m();
			Rect r = bb2Rect(bbScale(m_bbDraw, pM->cols, pM->rows));

			Mat m;
			cv::resize(*m_fBGR.m(),m,Size(r.width,r.height));

			m.copyTo((*pFrame->m())(r));
		}
	}

	this->_ModuleBase::draw();
}

}
#endif
