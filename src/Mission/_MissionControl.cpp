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

		ADD_MISSION(MissionBase);
		ADD_MISSION(Waypoint);
		ADD_MISSION(Land);

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
	F_INFO(pK->v("startMission", &startMission));
	m_iMission = getMissionIdx(startMission);

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

		IF_CONT(check() < 0);
		IF_CONT(m_iMission < 0);
		IF_CONT(m_iMission >= m_vMission.size());

		MissionBase* pMission = m_vMission[m_iMission].m_pInst;

		if(pMission->update())
		{
			m_iMission = getMissionIdx(pMission->m_nextMission);
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

bool _MissionControl::transit(int iNextMission)
{
	m_iMission = iNextMission;
	return true;
}

MissionBase* _MissionControl::getCurrentMission(void)
{
	IF_N(m_iMission >= m_vMission.size());
	IF_N(m_iMission < 0);
	return m_vMission[m_iMission].m_pInst;
}

int _MissionControl::getCurrentMissionIdx(void)
{
	return m_iMission;
}

string* _MissionControl::getCurrentMissionName(void)
{
	IF_N(m_iMission >= m_vMission.size());
	IF_N(m_iMission < 0);
	static string mName = ((Kiss*)m_vMission[m_iMission].m_pInst->m_pKiss)->m_name;
	return &mName;
}

bool _MissionControl::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	pWin->addMsg(*this->getName());

	pWin->addMsg("nMission: "+i2str(m_vMission.size()));
	IF_T(m_vMission.size() <= 0);

	pWin->addMsg("iMission: "+i2str(m_iMission));
	IF_T(m_iMission < 0);

	MissionBase* pMB = m_vMission[m_iMission].m_pInst;
	pWin->addMsg("Current mission: " + ((Kiss*)pMB->m_pKiss)->m_name);

	pWin->tabNext();
	pMB->draw();
	pWin->tabPrev();

	return true;
}

bool _MissionControl::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));
	string msg;

	C_MSG("nMission: "+i2str(m_vMission.size()));
	IF_T(m_vMission.size() <= 0);

	C_MSG("iMission: "+i2str(m_iMission));
	IF_T(m_iMission < 0);

	MissionBase* pMB = m_vMission[m_iMission].m_pInst;
	C_MSG("Current mission: " + ((Kiss*)pMB->m_pKiss)->m_name);

	iY++;
	pMB->console(iY);

	return true;
}

}
