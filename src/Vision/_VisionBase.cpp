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
	m_type = vision_unknown;
	m_w = 1280;
	m_h = 720;
	m_cW = 640;
	m_cH = 360;
	m_fovW = 60;
	m_fovH = 60;
	m_bbDraw.x = -1.0;
}

_VisionBase::~_VisionBase()
{
}

bool _VisionBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("w",&m_w);
	pK->v("h",&m_h);
	m_cW = m_w / 2;
	m_cH = m_h / 2;

	pK->v("fovW",&m_fovW);
	pK->v("fovH",&m_fovH);
	pK->v("bbDraw",&m_bbDraw);

	m_bOpen = false;
	return true;
}

Frame* _VisionBase::BGR(void)
{
	return &m_fBGR;
}

void _VisionBase::info(vInt2* pSize, vInt2* pCenter, vDouble2* pAngle)
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

vInt2 _VisionBase::getSize(void)
{
	vInt2 s;
	s.x = m_w;
	s.y = m_h;
	return s;
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
	m_bOpen = false;
}

void _VisionBase::draw(void)
{
	NULL_(m_pWindow);
	Frame* pFrame = ((Window*)m_pWindow)->getFrame();

	if(!m_fBGR.bEmpty())
	{
		if(m_bbDraw.x < 0.0)
		{
			pFrame->copy(m_fBGR);
		}
		else
		{
			Mat* pM = pFrame->m();

			vInt2 cs;
			cs.x = pM->cols;
			cs.y = pM->rows;
			Rect r = convertBB<vInt4>(convertBB(m_bbDraw, cs));

			Mat m;
			cv::resize(*m_fBGR.m(),m,Size(r.width,r.height));

			m.copyTo((*pFrame->m())(r));
		}
	}

	this->_ThreadBase::draw();
}

}
