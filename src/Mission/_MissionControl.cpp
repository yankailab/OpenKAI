/*
 * _MissionControl.cpp
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai
 */

#include "_MissionControl.h"

namespace kai
{

_MissionControl::_MissionControl()
{
	m_iMission = -1;
}

_MissionControl::~_MissionControl()
{
	for(unsigned int i=0; i<m_vMission.size(); i++)
	{
		DEL(m_vMission[i].m_pInst);
	}

	m_vMission.clear();
}

bool _MissionControl::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	Kiss* pCC = pK->o("mission");
	IF_T(pCC->empty());
	Kiss** pItr = pCC->getChildItr();

	MISSION_INST M;
	unsigned int i = 0;
	while (pItr[i])
	{
		Kiss* pKM = pItr[i++];

		bool bInst = false;
		pKM->v("bInst", &bInst);
		IF_CONT(!bInst);

		M.init();

		//Add action modules below

		ADD_MISSION(Waypoint);

		//Add action modules above

		IF_Fl(!M.m_pInst, "Unknown mission class: "+pKM->m_class);

		pKM->m_pInst = M.m_pInst;
		m_vMission.push_back(M);
	}

	for(i=0; i<m_vMission.size();i++)
	{
		MISSION_INST* pM = &m_vMission[i];
		IF_Fl(!pM->m_pInst->init(pM->m_pKiss), pM->m_pKiss->m_name+": init failed");
	}

	string startMission = "";
	F_ERROR_F(pK->v("startMission", &startMission));
	m_iMission = getMissionIdx(startMission);
	IF_F(m_iMission < 0);

	return true;
}

bool _MissionControl::start(void)
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

int _MissionControl::check(void)
{
	return 0;
}

void _MissionControl::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		IF_CONT(check()<0);
		IF_CONT(m_iMission<0);

		MissionBase* pMission = m_vMission[m_iMission].m_pInst;

		if(pMission->update())
		{
			//TODO:transit to next Mission
		}

		this->autoFPSto();
	}
}

int _MissionControl::getMissionIdx(const string& missionName)
{
	for(int i=0;i<m_vMission.size();i++)
	{
		if(((Kiss*)m_vMission[i].m_pInst->m_pKiss)->m_name == missionName)
			return i;
	}

	return -1;
}

bool _MissionControl::transit(const string& nextMissionName)
{
	int iNext = getMissionIdx(nextMissionName);
	return transit(iNext);
}

bool _MissionControl::transit(int nextMissionIdx)
{
	IF_F(nextMissionIdx < 0);
	IF_F(nextMissionIdx >= m_vMission.size());
	IF_F(nextMissionIdx == m_iMission);

	m_iMission = nextMissionIdx;
	return true;
}

MissionBase* _MissionControl::getCurrentMission(void)
{
	IF_N(m_iMission >= m_vMission.size());
	return m_vMission[m_iMission].m_pInst;
}

int _MissionControl::getCurrentMissionIdx(void)
{
	return m_iMission;
}

string* _MissionControl::getCurrentMissionName(void)
{
	IF_N(m_iMission >= m_vMission.size());
	static string currentMissionName = ((Kiss*)m_vMission[m_iMission].m_pInst->m_pKiss)->m_name;
	return &currentMissionName;
}

bool _MissionControl::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	IF_T(m_vMission.size() <= 0);
	IF_T(m_iMission < 0);

	string msg = *this->getName()+": " + ((Kiss*)m_vMission[m_iMission].m_pInst->m_pKiss)->m_name;
	pWin->addMsg(&msg);

	pWin->tabNext();
	for(unsigned int i=0; i<m_vMission.size(); i++)
	{
		m_vMission[i].m_pInst->draw();
	}
	pWin->tabPrev();

	return true;
}

bool _MissionControl::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	IF_T(m_vMission.size() <= 0);
	IF_T(m_iMission < 0);

	string msg = "Current mission: " + ((Kiss*)m_vMission[m_iMission].m_pInst->m_pKiss)->m_name;
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	for(unsigned int i=0; i<m_vMission.size(); i++)
	{
		iY++;
		m_vMission[i].m_pInst->console(iY);
	}

	return true;
}

}
