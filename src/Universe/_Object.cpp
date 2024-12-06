/*
 *  Created on: June 21, 2019
 *      Author: yankai
 */
#include "_Object.h"

namespace kai
{

	_Object::_Object()
	{
		clear();
	}

	_Object::~_Object()
	{
	}

	int _Object::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	void _Object::clear(void)
	{
		m_vPos.clear();
		m_vAtti.clear();
		m_vDim.clear();
		m_vSpeed.clear();
		m_vAccel.clear();
		m_mFlag = 0;
		m_pTracker = NULL;
		m_tStamp = 0;

		resetClass();
	}

	int _Object::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _Object::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

		}
	}

	void _Object::updateKinetics(void)
	{
		IF_(check() != OK_OK);
		m_vSpeed += m_vAccel * m_pT->getDt() * USEC_2_SEC;
		m_vPos += m_vSpeed;
	}

	void _Object::setPos(vFloat3 &p)
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

	void _Object::setPosScr(vFloat2 &p)
	{
		m_vPosScr = p;
	}

	vFloat2 _Object::getPosScr(void)
	{
		return m_vPosScr;
	}

	void _Object::setDimScr(vFloat2 &d)
	{
		m_vDimScr = d;
	}

	vFloat2 _Object::getDimScr(void)
	{
		return m_vDimScr;
	}

	void _Object::setAttitude(vFloat3 &a)
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

	void _Object::setDim(vFloat4 &d)
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
			vFloat2 *v = &pV[i];
			p.x = v->x;
			p.y = v->y;
			vP.push_back(p);
			m_vVertex.push_back(*v);
		}

		vFloat4 bb = rect2BB<vFloat4>(boundingRect(vP));
		setBB2D(bb);
#endif
	}

	vFloat2 *_Object::getVertex(int i)
	{
		IF__(i > m_vVertex.size(), nullptr);

		return &m_vVertex[i];
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

	void _Object::setText(string &txt)
	{
		m_txt = txt;
	}

	string _Object::getText(void)
	{
		return m_txt;
	}

	void _Object::setTstamp(uint64_t t)
	{
		m_tStamp = t;
	}

	uint64_t _Object::getTstamp(void)
	{
		return m_tStamp;
	}

}
