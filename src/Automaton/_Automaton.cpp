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

	m_nState = 0;
	m_pMyState = NULL;

}

_Automaton::~_Automaton()
{
	// TODO Auto-generated destructor stub
}

bool _Automaton::init(Kiss* pKiss)
{
	CHECK_F(this->_ThreadBase::init(pKiss)==false);

	pKiss->m_pInst = this;

	//create state instances
	Kiss** pItr = pKiss->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Kiss* pState = pItr[i];
		i++;

		bool bInst = false;
		F_INFO(pState->v("bInst", &bInst));
		if (!bInst)continue;
		if (pState->m_class != "State")continue;
		CHECK_F(m_nState >= N_STATE);

		State** ppS = &m_pState[m_nState];
		m_nState++;

		*ppS = new State();
		F_ERROR_F((*ppS)->init(pState));
	}

	//link state instances
	for(i=0;i<m_nState;i++)
	{
		F_ERROR_F(m_pState[i]->link(pKiss));
	}

	string startState = "";
	F_FATAL_F(pKiss->v("startState", &startState));
	m_pMyState = (State*)(pKiss->getChildInstByName(&startState));
	NULL_F(m_pMyState);

	return true;
}

bool _Automaton::link(void)
{
	NULL_F(m_pKiss);

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
	State* pNext = m_pMyState->Transit();

	NULL_(pNext);

	m_pMyState = pNext;
}

bool _Automaton::setPtrByName(string name, int* ptr)
{
	NULL_F(ptr);
	CHECK_F(name == "");

	for (int i = 0; i < m_nState; i++)
	{
		m_pState[i]->setPtrByName(name, ptr);
	}

	return true;
}

bool _Automaton::setPtrByName(string name, double* ptr)
{
	NULL_F(ptr);
	CHECK_F(name == "");

	for (int i = 0; i < m_nState; i++)
	{
		m_pState[i]->setPtrByName(name, ptr);
	}

	return true;
}

bool _Automaton::checkDiagram(void)
{
	return true;
}

bool _Automaton::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);

	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "Automaton FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	putText(*pMat,
			"Automaton State: " + m_pMyState->m_name,
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

} /* namespace kai */
