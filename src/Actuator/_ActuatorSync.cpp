/*
 *  Created on: May 16, 2019
 *      Author: yankai
 */
#include "_ActuatorSync.h"

namespace kai
{

_ActuatorSync::_ActuatorSync()
{
	m_nAB = 0;
	m_iABget = 0;
}

_ActuatorSync::~_ActuatorSync()
{
}

bool _ActuatorSync::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	vector<string> vName;
	pK->a("_ActuatorList", &vName);

	for(int i=0; i<vName.size(); i++)
	{
		void* p = pK->getInst(vName[i]);
		IF_CONT(!p);

		m_pAB[m_nAB] = (_ActuatorSync*)p;
		m_nAB++;
	}

	pK->v("iABget",&m_iABget);
	if(m_iABget >= m_nAB)
		m_iABget = 0;

	return true;
}

bool _ActuatorSync::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _ActuatorSync::check(void)
{
	return this->_ActuatorBase::check();
}

void _ActuatorSync::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateSync();

		this->autoFPSto();
	}
}

void _ActuatorSync::updateSync(void)
{
	_ActuatorBase* pA = m_pAB[m_iABget];
	NULL_(pA);
	m_vNormPos = pA->getPos();
	m_vNormSpeed = pA->getSpeed();

}

void _ActuatorSync::speed(vFloat4& vSpeed)
{
	for(int i=0; i<m_nAB; i++)
		m_pAB[i]->speed(vSpeed);

	this->_ActuatorBase::speed(vSpeed);
}

void _ActuatorSync::pos(vFloat4& vPos)
{
	for(int i=0; i<m_nAB; i++)
	{
		m_pAB[i]->pos(vPos);
	}

	this->_ActuatorBase::pos(vPos);
}

void _ActuatorSync::gotoOrigin(void)
{
	for(int i=0; i<m_nAB; i++)
		m_pAB[i]->gotoOrigin();

	this->_ActuatorBase::gotoOrigin();
}

bool _ActuatorSync::bComplete(void)
{
	for(int i=0; i<m_nAB; i++)
	{
		IF_F(!m_pAB[i]->bComplete());
	}

	return true;
}

}
