/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_MultiActuatorsBase.h"

namespace kai
{

	_MultiActuatorsBase::_MultiActuatorsBase()
	{
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

		for (int i = 0; i < vName.size(); i++)
		{
			void *p = pK->findModule(vName[i]);
			IF_CONT(!p);

			m_vAB.push_back((_ActuatorBase *)p);
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

	void _MultiActuatorsBase::setPtargetAll(float nP)
	{
		for (_ActuatorBase* pA : m_vAB)
			pA->getChan()->pos()->setTarget(nP);
	}

	void _MultiActuatorsBase::setStargetAll(float nS)
	{
		for (_ActuatorBase* pA : m_vAB)
			pA->getChan()->speed()->setTarget(nS);
	}

	void _MultiActuatorsBase::gotoOriginAll(void)
	{
		for (_ActuatorBase* pA : m_vAB)
			pA->getChan()->gotoOrigin();
	}

	bool _MultiActuatorsBase::bCompleteAll(void)
	{
		for (_ActuatorBase* pA : m_vAB)
		{
			IF_F(!pA->getChan()->bComplete());
		}

		return true;
	}

}
