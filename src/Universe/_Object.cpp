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

void _Object::setPos(vFloat3& p)
{
	m_vPos = p;
}

vFloat3 _Object::getPos(void)
{
	return m_vPos;
}

void _Object::setAttitude(vFloat3& a)
{
	m_vAtti = a;
}

vFloat3 _Object::getAttitude(void)
{
	return m_vAtti;
}

void _Object::setDim(vFloat4& d)
{
	m_vDim = d;
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

void _Object::setR(float r)
{
	m_vDim.w = r;
}

vFloat4 _Object::getDim(void)
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

float _Object::getR(void)
{
	return m_vDim.w;
}

float _Object::area(void)
{
	return m_vDim.x * m_vDim.y;
}

float _Object::volume(void)
{
	return m_vDim.x * m_vDim.y * m_vDim.z;
}

void _Object::setBB2D(vFloat4& bb)
{
	m_vPos.x = (bb.x + bb.z) * 0.5;
	m_vPos.y = (bb.y + bb.w) * 0.5;

	m_vDim.x = bb.width();
	m_vDim.y = bb.height();
}

vFloat4 _Object::getBB2D(void)
{
	float hw = m_vDim.x * 0.5;
	float hh = m_vDim.y * 0.5;

	vFloat4 bb;
	bb.x = m_vPos.x - hw;
	bb.y = m_vPos.y - hh;
	bb.z = m_vPos.x + hw;
	bb.w = m_vPos.y + hh;

	return bb;
}

vFloat4 _Object::getBB2Dnorm(float nx, float ny)
{
	float hw = m_vDim.x * 0.5;
	float hh = m_vDim.y * 0.5;

	vFloat4 bb;
	bb.x = (m_vPos.x - hw) * nx;
	bb.y = (m_vPos.y - hh) * ny;
	bb.z = (m_vPos.x + hw) * nx;
	bb.w = (m_vPos.y + hh) * ny;

	return bb;
}

void _Object::setVertices2D(vFloat2 *pV, int nV)
{
	NULL_(pV);

	m_vVertex.clear();
	vector<Point> vP;
	Point p;
	for (int i = 0; i < nV; i++)
	{
		vFloat2* v = &pV[i];
		p.x = v->x;
		p.y = v->y;
		vP.push_back(p);
		m_vVertex.push_back(*v);
	}

	vFloat4 bb = convertBB<vFloat4>(boundingRect(vP));
	setBB2D(bb);
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

float _Object::nIoU(_Object& obj)
{

	return 0.0;
}

void _Object::draw(void)
{
	this->_ThreadBase::draw();

//	IF_(!checkWindow());
//	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();
}

}
