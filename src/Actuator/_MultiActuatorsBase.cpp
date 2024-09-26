/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_MultiActuatorsBase.h"

namespace kai
{

	_MultiActuatorsBase::_MultiActuatorsBase()
	{
		m_nAB = 0;
		m_bPower = false;
		m_bReady = false;
	}

	_MultiActuatorsBase::~_MultiActuatorsBase()
	{
	}

	int _MultiActuatorsBase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _MultiActuatorsBase::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;

		vector<string> vName;
		pK->a("vActuatorBase", &vName);
		m_nAB = 0;

		for (int i = 0; i < vName.size(); i++)
		{
			void *p = pK->findModule(vName[i]);
			IF_CONT(!p);

			m_pAB[m_nAB] = (_ActuatorBase *)p;
			m_nAB++;
		}

		return OK_OK;
	}

	int _MultiActuatorsBase::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _MultiActuatorsBase::check(void)
	{
		return this->_ModuleBase::check();
	}

	void _MultiActuatorsBase::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateActuators();

			m_pT->autoFPSto();
		}
	}

	void _MultiActuatorsBase::updateActuators(void)
	{
	}

	void _MultiActuatorsBase::setPtarget(int iA, float nP)
	{
		IF_(iA < 0 || iA >= m_nAB);

		m_pAB[iA]->setPtarget(nP);
	}

	void _MultiActuatorsBase::setStarget(int iA, float nS)
	{
		IF_(iA < 0 || iA >= m_nAB);

		m_pAB[iA]->setStarget(nS);
	}

	void _MultiActuatorsBase::gotoOrigin(int iA)
	{
		IF_(iA < 0 || iA >= m_nAB);

		m_pAB[iA]->gotoOrigin();
	}

	bool _MultiActuatorsBase::bComplete(int iA)
	{
		IF_F(iA < 0 || iA >= m_nAB);

		IF_F(!m_pAB[iA]->bComplete());
		return true;
	}

	void _MultiActuatorsBase::setPtargetAll(float nP)
	{
		for (int i = 0; i < m_nAB; i++)
			m_pAB[i]->setPtarget(nP);
	}

	void _MultiActuatorsBase::setStargetAll(float nS)
	{
		for (int i = 0; i < m_nAB; i++)
			m_pAB[i]->setStarget(nS);
	}

	void _MultiActuatorsBase::gotoOriginAll(void)
	{
		for (int i = 0; i < m_nAB; i++)
			m_pAB[i]->gotoOrigin();
	}

	bool _MultiActuatorsBase::bCompleteAll(void)
	{
		for (int i = 0; i < m_nAB; i++)
		{
			IF_F(!m_pAB[i]->bComplete());
		}

		return true;
	}

}
