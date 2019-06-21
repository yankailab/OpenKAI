/*
 *  Created on: Sept 28, 2016
 *      Author: yankai
 */
#include "_Object.h"

namespace kai
{

_Object::_Object()
{
	m_p.init();
	m_a.init();
	m_q.init();
	m_bb.init();

	resetClass();
}

_Object::~_Object()
{
}

bool _Object::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _Object::start(void)
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

void _Object::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->autoFPSto();
	}
}

void _Object::updateKinetics(void)
{
	m_speed += m_accel * (float)m_dTime * OV_USEC_1SEC;
	m_p += m_speed;
}

void _Object::setSize(vFloat3 s)
{
	m_bb.x = s.x;
	m_bb.y = s.y;
	m_bb.z = s.z;
}

void _Object::setLength(float l)
{
	m_bb.x = l;
}

void _Object::setWidth(float w)
{
	m_bb.y = w;
}

void _Object::setHeight(float h)
{
	m_bb.z = h;
}

void _Object::setRadius(float r)
{
	m_bb.w = r;
}

vFloat3 _Object::getSize(void)
{
	vFloat3 v;
	v.x = m_bb.x;
	v.y = m_bb.y;
	v.z = m_bb.z;

	return v;
}

float _Object::getLength(void)
{
	return m_bb.x;
}

float _Object::getWidth(void)
{
	return m_bb.y;
}

float _Object::getHeight(void)
{
	return m_bb.z;
}

float _Object::getRadius(void)
{
	return m_bb.w;
}

float _Object::area(void)
{
	return m_bb.x * m_bb.y;
}

float _Object::volume(void)
{
	return m_bb.x * m_bb.y * m_bb.z;
}

void _Object::addClassIdx(int iClass)
{
	m_mClass |= 1 << iClass;
}

void _Object::setClassMask(uint64_t mClass)
{
	m_mClass = mClass;
}

void _Object::setTopClass(int iClass, float prob)
{
	m_topClass = iClass;
	m_topProb = prob;
	addClassIdx(iClass);
}

bool _Object::bClass(int iClass)
{
	return (m_mClass & (1 << iClass));
}

bool _Object::bClassMask(uint64_t mClass)
{
	return m_mClass & mClass;
}

void _Object::resetClass(void)
{
	m_txt = "";
	m_topClass = -1;
	m_topProb = 0.0;
	m_mClass = 0;
}

bool _Object::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	return true;
}

bool _Object::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
