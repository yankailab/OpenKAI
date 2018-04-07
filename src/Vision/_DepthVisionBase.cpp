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
	m_pDepth = NULL;
	m_pDepthWin = NULL;
	m_pDepthShow = NULL;
	m_range.x = 0.0;
	m_range.y = 10.0;
	m_wD = 1280;
	m_hD = 720;
}

_DepthVisionBase::~_DepthVisionBase()
{
	reset();
}

bool _DepthVisionBase::init(void* pKiss)
{
	IF_F(!this->_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,wD);
	KISSm(pK,hD);
	F_INFO(pK->v("rFrom", &m_range.x));
	F_INFO(pK->v("rTo", &m_range.y));

	m_pDepth = new Frame();

	return true;
}

bool _DepthVisionBase::link(void)
{
	IF_F(!this->_VisionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("depthWindow", &iName));
	m_pDepthWin = (Window*) (pK->root()->getChildInstByName(&iName));
	if(m_pDepthWin)
	{
		m_pDepthShow = new Frame();
	}

	return true;
}

void _DepthVisionBase::reset(void)
{
	this->_VisionBase::reset();

	DEL(m_pDepth);
}

Frame* _DepthVisionBase::depth(void)
{
	return m_pDepth;
}

vDouble2 _DepthVisionBase::range(void)
{
	return m_range;
}

bool _DepthVisionBase::draw(void)
{
	IF_F(!this->_VisionBase::draw());
	NULL_F(m_pDepthWin);
	NULL_F(m_pDepthShow);
	IF_F(m_pDepthShow->empty());

	Frame* pFrame = m_pDepthWin->getFrame();
	pFrame->update(m_pDepthShow);

	return true;
}


}
