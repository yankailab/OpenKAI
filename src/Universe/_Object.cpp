/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_Object.h"

namespace kai
{

_Object::_Object()
{
	init();
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

void _Object::init(void)
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

void _Object::setX(float x)
{
	m_vPos.x = x;
}

void _Object::setY(float y)
{
	m_vPos.y = y;
}

void _Object::setZ(float z)
{
	m_vPos.z = z;
}

float _Object::getX(void)
{
	return m_vPos.x;
}

float _Object::getY(void)
{
	return m_vPos.y;
}

float _Object::getZ(void)
{
	return m_vPos.z;
}

void _Object::setAttitude(vFloat3& a)
{
	m_vAtti = a;
}

vFloat3 _Object::getAttitude(void)
{
	return m_vAtti;
}

void _Object::setRoll(float r)
{
	m_vAtti.x = r;
}

void _Object::setPitch(float p)
{
	m_vAtti.y = p;
}

void _Object::setYaw(float y)
{
	m_vAtti.z = y;
}

float _Object::getRoll(void)
{
	return m_vAtti.x;
}

float _Object::getPitch(void)
{
	return m_vAtti.y;
}

float _Object::getYaw(void)
{
	return m_vAtti.z;
}

void _Object::setDim(vFloat4& d)
{
	m_vDim = d;
}

void _Object::setWidth(float w)
{
	m_vDim.x = w;
}

void _Object::setHeight(float h)
{
	m_vDim.y = h;
}

void _Object::setDepth(float d)
{
	m_vDim.z = d;
}

void _Object::setRadius(float r)
{
	m_vDim.w = r;
}

vFloat4 _Object::getDim(void)
{
	return m_vDim;
}

float _Object::getWidth(void)
{
	return m_vDim.x;
}

float _Object::getHeight(void)
{
	return m_vDim.y;
}

float _Object::getDepth(void)
{
	return m_vDim.z;
}

float _Object::getRadius(void)
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

void _Object::setBB2D(float l, float t, float w, float h)
{
	m_vPos.x = l + w * 0.5;
	m_vPos.y = t + h * 0.5;

	m_vDim.x = w;
	m_vDim.y = h;
}

void _Object::setBB2D(vFloat4 bb)
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

vFloat4 _Object::getBB2Dscaled(float kx, float ky)
{
	float hw = m_vDim.x * 0.5;
	float hh = m_vDim.y * 0.5;

	vFloat4 bb;
	bb.x = (m_vPos.x - hw) * kx;
	bb.y = (m_vPos.y - hh) * ky;
	bb.z = (m_vPos.x + hw) * kx;
	bb.w = (m_vPos.y + hh) * ky;

	return bb;
}

void _Object::scale(float kx, float ky, float kz)
{
	m_vPos.x *= kx;
	m_vPos.y *= ky;
	m_vPos.z *= kz;

	m_vDim.x *= kx;
	m_vDim.y *= ky;
	m_vDim.z *= kz;
	m_vDim.w *= kx;
}

void _Object::setVertices2D(vFloat2 *pV, int nV)
{
	NULL_(pV);

#ifdef USE_OPENCV
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

	vFloat4 bb = rect2BB<vFloat4>(boundingRect(vP));
	setBB2D(bb);
#endif
}

vFloat2* _Object::getVertex(int i)
{
	IF_N(i > m_vVertex.size());

	return &m_vVertex[i];
}

void _Object::setPointCloudPoint(vector<Eigen::Vector3d>& vP)
{
	m_vPCpoint = vP;
}

void _Object::setPointCloudColor(vector<Eigen::Vector3d>& vC)
{
	m_vPCcolor = vC;
}

vector<Eigen::Vector3d>* _Object::getPointCloudPoint(void)
{
	return &m_vPCpoint;
}

vector<Eigen::Vector3d>* _Object::getPointCloudColor(void)
{
	return &m_vPCcolor;
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

int _Object::getTopClass(void)
{
	return m_topClass;
}

float _Object::getTopClassProb(void)
{
	return m_topProb;
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

void _Object::setText(string & txt)
{
	m_txt = txt;
}

string _Object::getText(void)
{
	return m_txt;
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
