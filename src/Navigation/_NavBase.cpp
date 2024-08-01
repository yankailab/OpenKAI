/*
 * _NavBase.cpp
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#include "_NavBase.h"

namespace kai
{

	_NavBase::_NavBase()
	{
		m_scale = 1.0;
		m_flag.clearAll();
		m_vAxisIdx.set(0, 1, 2);
		m_vRoffset.clear();
		resetAll();
	}

	_NavBase::~_NavBase()
	{
	}

	int _NavBase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("scale", &m_scale);
		pK->v("vAxisIdx", &m_vAxisIdx);
		pK->v("vRoffset", &m_vRoffset);

		return OK_OK;
	}

	bool _NavBase::bOpen(void)
	{
		return m_flag.b(F_OPEN);
	}

	bool _NavBase::bError(void)
	{
		return m_flag.b(F_ERROR);
	}

	void _NavBase::reset(void)
	{
		m_flag.set(F_RESET);
	}

	float _NavBase::confidence(void)
	{
		return m_confidence;
	}

	void _NavBase::resetAll(void)
	{
		m_vT.clear();
		m_vV.clear();
		m_vR.clear();
		m_vQ.clear();
		m_mT = Matrix4f::Identity();
		m_confidence = 0.0;
	}

	vFloat3 _NavBase::t(void)
	{
		return m_vT;
	}

	vFloat3 _NavBase::v(void)
	{
		return m_vV;
	}

	vFloat3 _NavBase::r(void)
	{
		return m_vR;
	}

	vFloat4 _NavBase::q(void)
	{
		return m_vQ;
	}

	const Matrix4f &_NavBase::mT(void)
	{
		return m_mT;
	}

	const Matrix3f &_NavBase::mR(void)
	{
		return m_mR;
	}

	void _NavBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		msg = "vT = (" + f2str(m_vT.x, 3) + ", " + f2str(m_vT.y, 3) + ", " + f2str(m_vT.z, 3) + ")";
		pC->addMsg(msg, 1);

		msg = "vV = (" + f2str(m_vV.x, 3) + ", " + f2str(m_vV.y, 3) + ", " + f2str(m_vV.z, 3) + ")";
		pC->addMsg(msg, 1);

		msg = "vR = (" + f2str(m_vR.x, 3) + ", " + f2str(m_vR.y, 3) + ", " + f2str(m_vR.z, 3) + ")";
		pC->addMsg(msg, 1);

		msg = "vQ = (" + f2str(m_vQ.x, 3) + ", " + f2str(m_vQ.y, 3) + ", " + f2str(m_vQ.z, 3) + ", " + f2str(m_vQ.w, 3) + ")";
		pC->addMsg(msg, 1);

		msg = "confidence=" + f2str(m_confidence);
		pC->addMsg(msg, 1);
	}

}
