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
		void* p = pK->root()->getChildInst(vName[i]);
		IF_CONT(!p);

		m_pAB[m_nAB] = (_ActuatorSync*)p;
		m_nAB++;
	}

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

void _ActuatorSync::update(void)
{
}

void _ActuatorSync::move(vFloat4& vSpeed)
{
	for(int i=0; i<m_nAB; i++)
		m_pAB[i]->move(vSpeed);
}

void _ActuatorSync::moveTo(vFloat4& vPos, vFloat4& vSpeed)
{
	for(int i=0; i<m_nAB; i++)
		m_pAB[i]->moveTo(vPos, vSpeed);
}

void _ActuatorSync::moveToOrigin(void)
{
	for(int i=0; i<m_nAB; i++)
		m_pAB[i]->moveToOrigin();
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
