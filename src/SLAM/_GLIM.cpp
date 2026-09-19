/*
 * _GLIM.cpp
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#include "_GLIM.h"

namespace kai
{

	_GLIM::_GLIM()
	{
	}

	_GLIM::~_GLIM()
	{
	}

	bool _GLIM::init(const json &j)
	{
		IF_F(!this->_SLAMbase::init(j));


		return true;
	}

	bool _GLIM::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_SLAMbase::link(j, pM));

		string n;

		n = "";
		jKv(j, "_PointCloud", n);
		m_pPCL = (_PointCloud *)(pM->findModule(n));
		IF_Le_F(!m_pPCL, "Cannot find _PointCloud: " + n);

		return true;
	}

	bool _GLIM::check(void)
	{
		NULL_F(m_pPCL);

		return this->_SLAMbase::check();
	}

	bool _GLIM::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _GLIM::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
		}
	}

	void _GLIM::updateGLIM(void)
	{
		
	}

	void _GLIM::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_SLAMbase::console(pConsole);

//		_Console *pC = (_Console *)pConsole;
//		pC->addMsg(msg);

	}

}
