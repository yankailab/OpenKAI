/*
 * State.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: root
 */

#include "_State.h"

namespace kai
{

_State::_State()
{
	_ThreadBase();

	m_numState = 0;

}

_State::~_State()
{
	// TODO Auto-generated destructor stub
}

bool _State::init(JSON* pJson, string stateName)
{
	if (!pJson)return false;

	double FPS = DEFAULT_FPS;
	CHECK_INFO(pJson->getVal("STATE_"+stateName+"_FPS", &FPS));
	this->setTargetFPS(FPS);

	int i;
	string fullName;
	string name;
	string stateID;
	string stateTransitionID;
	string stateTransitionCondID;
	STATE* pS;
	STATE_TRANSITION* pST;
	STATE_TRANSITION_COND* pSTC;

	fullName = "STATE_"+stateName+"_STATE_";

	m_numState = 0;
	stateID = i2str(m_numState);

	while(pJson->getVal(fullName+stateID, &FPS))
	{
		name = "";
		CHECK_INFO(pJson->getVal(fullName+stateID+"_NAME", &name));

		pS = addState(name);
		if(pS==NULL)return false;

		stateTransitionID = i2str(pS->m_numTransition);
		pST = &pS->m_pTransition[pS->m_numTransition];

		while(pJson->getVal(fullName+stateID+"_TRANSITION_"+stateTransitionID+"_TO", &pST->m_transitToIdx))
		{

		}





		m_numState++;
		stateID = i2str(m_numState);
	}

	return true;
}

bool _State::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode << " in _State::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "AutoPilot.start()";

	return true;
}

void _State::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();


		this->autoFPSto();
	}

}

STATE* _State::addState(string name)
{

	return NULL;
}

STATE_TRANSITION* _State::addStateTransition(STATE* pState)
{

	return NULL;
}

STATE_TRANSITION_COND* _State::addStateTransitionCond(STATE_TRANSITION* pStateTransition)
{

	return NULL;
}


} /* namespace kai */
