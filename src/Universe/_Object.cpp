/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_Object.h"

namespace kai
{

_Object::_Object()
{
	m_vPos.init();
	m_vAtti.init();
	m_vDim.init();
	m_vSpeed.init();
	m_vAccel.init();
	m_vTraj.clear();
	m_mFlag = 0;
	m_pTracker = NULL;

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

void _Object::resetClass(void)
{
	m_txt = "";
	m_topClass = -1;
	m_topProb = 0.0;
	m_mClass = 0;
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
	m_vSpeed += m_vAccel * (float)m_dTime * OV_USEC_1SEC;
	m_vPos += m_vSpeed;
}

void _Object::setDim(vFloat3 s)
{
	m_vPos = s;
}

void _Object::setW(float w)
{
	m_vDim.x = w;
}

void _Object::setH(float h)
{
	m_vDim.y = h;
}

void _Object::setD(float d)
{
	m_vDim.z = d;
}

vFloat3 _Object::getDim(void)
{
	return m_vDim;
}

float _Object::getW(void)
{
	return m_vDim.x;
}

float _Object::getH(void)
{
	return m_vDim.y;
}

float _Object::getD(void)
{
	return m_vDim.z;
}

float _Object::area(void)
{
	return m_vDim.x * m_vDim.y;
}

float _Object::volume(void)
{
	return m_vDim.x * m_vDim.y * m_vDim.z;
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

float _Object::nIoU(_Object* pO)
{
	NULL__(pO,-1.0);

	return 0.0;
}

void _Object::draw(void)
{
	this->_ThreadBase::draw();

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();
}

}
