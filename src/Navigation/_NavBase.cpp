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
		m_flag.clearAll();
		m_vAxisIdx.init(0, 1, 2);
		m_vRoffset.init(0.0);
		resetAll();
	}

	_NavBase::~_NavBase()
	{
	}

	bool _NavBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vAxisIdx", &m_vAxisIdx);
		pK->v("vRoffset", &m_vRoffset);

		return true;
	}

	bool _NavBase::bOpen(void)
	{
		return m_flag.b(F_OPEN);
	}

	bool _NavBase::bReady(void)
	{
		return m_flag.b(F_READY);
	}

	void _NavBase::reset(void)
	{
		m_flag.clear(F_READY);
		m_flag.set(F_RESET);
	}

	float _NavBase::confidence(void)
	{
		return m_confidence;
	}

	void _NavBase::resetAll(void)
	{
		m_flag.clear(F_READY);
		m_vT.init();
		m_vV.init();
		m_vR.init();
		m_vQ.init();
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
#ifdef WITH_UI
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
#endif
	}

}
