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
	m_iState = 0;

}

_Automaton::~_Automaton()
{
	// TODO Auto-generated destructor stub
}

bool _Automaton::init(Config* pConfig, string name)
{
	if (this->_ThreadBase::init(pConfig,name)==false)
		return false;

	Config* pAM = pConfig->o(name);

	//init basic params
	m_numState = 0;

	//read in configuration
	string condStr;
	string typeStr;
	Config* cState;
	Config* cTransit;
	Config* cCond;

	do
	{
		cState = pAM->o("state" + i2str(m_numState));
		if (cState->empty())
			break;

		State* pS = addState();
		if (pS == NULL)
			return false;
		cState->v("stateName", &pS->m_name);

		do
		{
			cTransit = cState->o("transition" + i2str(pS->m_numTransition));
			if (cTransit->empty())
				break;

			Transition* pT = pS->addTransition();
			if (pT == NULL)
				return false;
			cTransit->v("toState", &pT->m_transitToID);

			do
			{
				cCond = cTransit->o("condition" + i2str(pT->m_numCond));
				if (cCond->empty())
					break;

				if (!cCond->v("type", &typeStr))
					break;
				if (!cCond->v("cond", &condStr))
					break;

				ConditionBase* pTC;

				if (typeStr == "ii")
				{
					pTC = pT->addConditionII();
				}
				else if (typeStr == "ff")
				{
					pTC = pT->addConditionFF();
				}
				else if (typeStr == "ic")
				{
					pTC = pT->addConditionIC();
					cCond->v("constI", &(((ConditionIC*) pTC)->m_const));
				}

				if (pTC == NULL)
					return false;

				if (condStr == "bt")
					pTC->m_condition = bt;
				else if (condStr == "beq")
					pTC->m_condition = beq;
				else if (condStr == "st")
					pTC->m_condition = st;
				else if (condStr == "seq")
					pTC->m_condition = seq;
				else if (condStr == "eq")
					pTC->m_condition = eq;
				else if (condStr == "neq")
					pTC->m_condition = neq;
				else
					pTC->m_condition = DEFAULT;

				cCond->v("ptrName1", &pTC->m_namePtr1);
				cCond->v("ptrName2", &pTC->m_namePtr2);

			} while (1);

		} while (1);

	} while (1);

	int k = 0;
	pAM->v("startState", &k);
	return setState(k);
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
	int iTransit = m_pState[m_iState]->Transit();

	if (iTransit < 0)
		return;
	if (iTransit >= m_numState)
		return;

	m_iState = iTransit;
}

bool _Automaton::setPtrByName(string name, int* ptr)
{
	if (ptr == NULL)
		return false;
	if (name == "")
		return false;

	for (int i = 0; i < m_numState; i++)
	{
		m_pState[i]->setPtrByName(name, ptr);
	}

	return true;
}

bool _Automaton::setPtrByName(string name, double* ptr)
{
	if (ptr == NULL)
		return false;
	if (name == "")
		return false;

	for (int i = 0; i < m_numState; i++)
	{
		m_pState[i]->setPtrByName(name, ptr);
	}

	return true;
}

State* _Automaton::addState(void)
{
	if (m_numState >= NUM_STATE)
		return NULL;

	State** ppS = &m_pState[m_numState];
	*ppS = new State();
	if (*ppS == NULL)
		return NULL;

	m_numState++;
	return *ppS;
}

bool _Automaton::setState(int iState)
{
	if (iState >= m_numState)
		return false;
	if (iState < 0)
		return false;

	m_iState = iState;
	return true;
}

bool _Automaton::checkDiagram(void)
{
	return true;
}

bool _Automaton::draw(Frame* pFrame, iVector4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	Mat* pMat = pFrame->getCMat();

	putText(*pFrame->getCMat(), "Automaton FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	putText(*pFrame->getCMat(),
			"Automaton State: " + m_pState[m_iState]->m_name + " ("
					+ i2str(m_iState) + ")",
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

} /* namespace kai */
