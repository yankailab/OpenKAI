/*
 * CamBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_StreamBase.h"

namespace kai
{

_StreamBase::_StreamBase()
{
	BASE();

	m_type = unknownStream;
	m_width = 1280;
	m_height = 720;
	m_centerH = 640;
	m_centerV = 360;
	m_bGimbal = false;
	m_isoScale = 1.0;
	m_rotTime = 0;
	m_rotPrev = 0;
	m_angleH = 0;
	m_angleV = 0;
}

_StreamBase::~_StreamBase()
{
}

bool _StreamBase::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("width", &m_width));
	F_INFO(pK->v("height", &m_height));
	F_INFO(pK->v("angleV", &m_angleV));
	F_INFO(pK->v("angleH", &m_angleH));
	F_INFO(pK->v("bGimbal", &m_bGimbal));
	F_INFO(pK->v("isoScale", &m_isoScale));

	return true;
}

Frame* _StreamBase::bgr(void)
{
	return NULL;
}

Frame* _StreamBase::hsv(void)
{
	return NULL;
}

Frame* _StreamBase::gray(void)
{
	return NULL;
}

Frame* _StreamBase::depth(void)
{
	return NULL;
}

void _StreamBase::setAttitude(double rollRad, double pitchRad, uint64_t timestamp)
{
	Point2f center(m_centerH, m_centerV);
	double deg = -rollRad * 180.0 * OneOvPI;

	m_rotRoll = getRotationMatrix2D(center, deg, m_isoScale);
	//TODO: add rot estimation

}

STREAM_TYPE _StreamBase::getType(void)
{
	return m_type;
}


}
