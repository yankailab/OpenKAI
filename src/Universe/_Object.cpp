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

	int _Object::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

		return true;
	}

	void _Object::clear(void)
	{
		m_type = obj_unknown;

		m_vPos.clear();
		m_vAtti.clear();
		m_vDim.clear();

		m_vSpeed.clear();
		m_vAccel.clear();

		m_pTracker = NULL;
		m_tStamp = 0;

		resetClass();
	}

	int _Object::start(void)
	{
		NULL_F(m_pT);
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
		IF_(!check());
		m_vSpeed += m_vAccel * m_pT->getDt() * USEC_2_SEC;
		m_vPos += m_vSpeed;
	}

	void _Object::setPos(const vFloat3 &vP)
	{
		m_vPos = vP;
	}

	void _Object::setPos(float x, float y, float z)
	{
		m_vPos.x = x;
		m_vPos.y = y;
		m_vPos.z = z;
	}

	vFloat3 _Object::getPos(void)
	{
		return m_vPos;
	}

	void _Object::setDim(const vFloat4 &vD)
	{
		m_vDim = vD;
	}

	void _Object::setDim(float w, float h, float d, float r)
	{
		m_vDim.x = w;
		m_vDim.y = h;
		m_vDim.z = d;
		m_vDim.w = r;
	}

	vFloat4 _Object::getDim(void)
	{
		return m_vDim;
	}

	float _Object::getDimArea(void)
	{
		return m_vDim.x * m_vDim.y;
	}

	float _Object::getDimVolume(void)
	{
		return m_vDim.x * m_vDim.y * m_vDim.z;
	}

	void _Object::setAttitude(const vFloat3 &vA)
	{
		m_vAtti = vA;
	}

	void _Object::setAttitude(float r, float p, float y)
	{
		m_vAtti.x = r;
		m_vAtti.y = p;
		m_vAtti.z = y;
	}

	vFloat3 _Object::getAttitude(void)
	{
		return m_vAtti;
	}

	void _Object::setBB2D(const vFloat4 &vBB, float kX, float kY)
	{
		m_vPos.x = (vBB.x + vBB.z) * 0.5 * kX;
		m_vPos.y = (vBB.y + vBB.w) * 0.5 * kY;

		m_vDim.x = (vBB.z - vBB.x) * kX;
		m_vDim.y = (vBB.w - vBB.y) * kY;
	}

	void _Object::setRect(float l, float t, float w, float h, float kX, float kY)
	{
		m_vPos.x = (l + w * 0.5) * kX;
		m_vPos.y = (t + h * 0.5) * kY;

		m_vDim.x = w * kX;
		m_vDim.y = h * kY;
	}

	vFloat4 _Object::getBB2D(float kX, float kY)
	{
		float hw = m_vDim.x * 0.5;
		float hh = m_vDim.y * 0.5;

		vFloat4 vBB;
		vBB.x = (m_vPos.x - hw) * kX;
		vBB.y = (m_vPos.y - hh) * kY;
		vBB.z = (m_vPos.x + hw) * kX;
		vBB.w = (m_vPos.y + hh) * kY;

		return vBB;
	}

	void _Object::setVertices2D(vFloat2 *pV, int nV, float kX, float kY)
	{
		NULL_(pV);

#ifdef USE_OPENCV
		m_vVertices.clear();

		vector<Point> vP;
		for (int i = 0; i < nV; i++)
		{
			vFloat2 *pVi = &pV[i];
			m_vVertices.push_back(vFloat2(pVi->x*kX, pVi->y*kY));
			vP.push_back(Point(pVi->x, pVi->y));
		}

		vFloat4 vBB = rect2BB<vFloat4>(boundingRect(vP));
		setBB2D(vBB, kX, kY);
#endif
	}

	vFloat2 *_Object::getVertex(int i)
	{
		IF__(i > m_vVertices.size(), nullptr);

		return &m_vVertices[i];
	}

	void _Object::setType(OBJ_TYPE type)
	{
		m_type = type;
	}

	OBJ_TYPE _Object::getType(void)
	{
		return m_type;
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
