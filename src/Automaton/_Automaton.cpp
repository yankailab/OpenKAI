/*
 * State.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: root
 */

#include "_Automaton.h"

namespace kai
{

_Automaton::_Automaton()
{
	_ThreadBase();

	m_numState = 0;
	m_iMyState = 0;

}

_Automaton::~_Automaton()
{
	// TODO Auto-generated destructor stub
}

bool _Automaton::init(JSON* pJson, string automatonName)
{
	if (!pJson)return false;

	double FPS = DEFAULT_FPS;
	CHECK_INFO(pJson->getVal("STATE_"+automatonName+"_FPS", &FPS));
	this->setTargetFPS(FPS);

	//init basic params
	m_numState = 0;

	//read in configuration
	int i,k;

	string name;
	string fullName;
	string stateID;
	string transitionID;
	string condID;
	string condStr;

	fullName = "STATE_"+automatonName+"_STATE_";
	stateID = i2str(m_numState);

	while(pJson->getVal(fullName+stateID+"_NAME", &name))
	{
		State* pS = addState();
		if(pS==NULL)return false;

		pS->m_name = name;
		transitionID = i2str(pS->m_numTransition);

		while(pJson->getVal(fullName+stateID+"_TRANSITION_"+transitionID+"_TO", &k))
		{
			Transition* pT = pS->addTransition();
			if(pT==NULL)return false;

			pT->m_transitToID = k;
			condID = i2str(pT->m_numCond);

			while(pJson->getVal(fullName+stateID+"_TRANSITION_"+transitionID+"_COND"+condID+"COND", &condStr))
			{
				TRANSITION_COND* pTC = pT->addCondition();
				if(pTC==NULL)return false;

				if(condStr=="bt")pTC->m_condition=bt;
				else if(condStr=="beq")pTC->m_condition=beq;
				else if(condStr=="st")pTC->m_condition=st;
				else if(condStr=="seq")pTC->m_condition=seq;
				else if(condStr=="eq")pTC->m_condition=eq;
				else if(condStr=="neq")pTC->m_condition=neq;

				pJson->getVal(fullName+stateID+"_TRANSITION_"+transitionID+"_COND"+condID+"NAME1", &pTC->m_namePtr1);
				pJson->getVal(fullName+stateID+"_TRANSITION_"+transitionID+"_COND"+condID+"NAME2", &pTC->m_namePtr2);
				if(!pJson->getVal(fullName+stateID+"_TRANSITION_"+transitionID+"_COND"+condID+"CONST2", &pTC->m_pFConst2))
				{
					pJson->getVal(fullName+stateID+"_TRANSITION_"+transitionID+"_COND"+condID+"CONST2", &pTC->m_pIConst2);
				}

				condID = i2str(pT->m_numCond);
			}

			transitionID = i2str(pS->m_numTransition);
		}

		stateID = i2str(m_numState);
	}

	return true;
}

bool _Automaton::start(void)
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

void _Automaton::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateAll();

		this->autoFPSto();
	}

}

void _Automaton::updateAll(void)
{

}


State* _Automaton::addState(void)
{

	return NULL;
}


} /* namespace kai */
