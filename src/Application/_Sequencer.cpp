/*
 * _Sequencer.cpp
 *
 *  Created on: May 26, 2019
 *      Author: yankai
 */

#include "_Sequencer.h"

namespace kai
{

_Sequencer::_Sequencer()
{
	m_nActuator = 0;
	m_iAction = 0;
}

_Sequencer::~_Sequencer()
{
}

bool _Sequencer::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	Kiss* pA;
	Kiss** pItr;
	string iName;

	pA = pK->o("actuator");
	if(pA)
	{
		pItr = pA->getChildItr();
		m_nActuator = 0;
		while (pItr[m_nActuator])
		{
			Kiss* pC = pItr[m_nActuator];

			iName = "";
			F_ERROR_F(pC->v("_ActuatorBase", &iName));
			m_ppActuator[m_nActuator] = (_ActuatorBase*) (pK->root()->getChildInst(iName));
			IF_Fl(!m_ppActuator[m_nActuator], iName + " not found");

			m_nActuator++;
		}
	}

	pA = pK->o("action");
	if(pA)
	{
		pItr = pA->getChildItr();
		SEQUENCER_ACTION a;
		int i=0;
		while (pItr[i])
		{
			Kiss* pC = pItr[i++];
			a.init();
			pC->v("name",&a.m_name);
			pC->v("dT", &a.m_dT);
			a.m_nA = pC->array("nPos", a.m_pNpos, SQ_N_ACTUATOR);
			m_vAction.push_back(a);
		}
	}

	return true;
}

bool _Sequencer::start(void)
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

void _Sequencer::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateAction();

		this->autoFPSto();
	}
}

int _Sequencer::check(void)
{
	return 0;
}

void _Sequencer::updateAction(void)
{
	IF_(m_nActuator<=0);
	IF_(m_vAction.size()<=0);

	int i;
	int nComplete = 0;
	SEQUENCER_ACTION* pAction = &m_vAction[m_iAction];
	for(i=0; i<pAction->m_nA; i++)
	{
		if(pAction->m_pNpos[i]<0.0)
		{
			nComplete++;
			continue;
		}

		m_ppActuator[i]->moveTo(pAction->m_pNpos[i], 1.0);
		if(m_ppActuator[i]->bComplete())
			nComplete++;
	}
	IF_(nComplete < pAction->m_nA);

	if(pAction->m_dT > 0)
	{
		this->sleepTime(pAction->m_dT);
	}
	else if(pAction->m_dT < 0)
	{
		this->sleepTime(0);
	}

	m_iAction++;
	if(m_iAction >= m_vAction.size())
		m_iAction = 0;
}

SEQUENCER_ACTION* _Sequencer::getAction(int iAction)
{
	IF_N(iAction >= m_vAction.size());
	return &m_vAction[iAction];
}

SEQUENCER_ACTION* _Sequencer::getAction(const string& name)
{
	for(int i=0; i<m_vAction.size(); i++)
	{
		SEQUENCER_ACTION* pA = &m_vAction[i];
		IF_CONT(pA->m_name != name);
		return pA;
	}

	return NULL;
}

SEQUENCER_ACTION* _Sequencer::getCurrentAction(void)
{
	return &m_vAction[m_iAction];
}

string _Sequencer::getCurrentActionName(void)
{
	return m_vAction[m_iAction].m_name;
}

bool _Sequencer::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _Sequencer::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
