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
	m_iMission = 0;
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
		M.init();

		//Add action modules below

		ADD_MISSION(MissionBase);
		ADD_MISSION(Waypoint);
		ADD_MISSION(Land);
		ADD_MISSION(Loiter);
		ADD_MISSION(Move);
		ADD_MISSION(RTH);

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
	if(m_iMission < 0)
		m_iMission = 0;

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

void _MissionControl::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		MissionBase* pMission = m_vMission[m_iMission].m_pInst;
		if(pMission->update())
		{
			transit(pMission->m_nextMission);
		}

		this->autoFPSto();
	}
}

void _MissionControl::transit(const string& nextMissionName)
{
	int iNext = getMissionIdx(nextMissionName);
	transit(iNext);
}

void _MissionControl::transit(int iNextMission)
{
	IF_(iNextMission < 0);
	IF_(iNextMission >= m_vMission.size());
	IF_(iNextMission == m_iMission);

	MissionBase* pMission = m_vMission[m_iMission].m_pInst;
	pMission->reset();

	m_iMission = iNextMission;
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

string _MissionControl::getCurrentMissionName(void)
{
	IF_N(m_iMission >= m_vMission.size());
	IF_N(m_iMission < 0);
	string mName = ((Kiss*)m_vMission[m_iMission].m_pInst->m_pKiss)->m_name;
	return mName;
}

void _MissionControl::draw(void)
{
	this->_ThreadBase::draw();

	addMsg("nMission: "+i2str(m_vMission.size()),1);
	IF_(m_vMission.size() <= 0);

	addMsg("iMission: "+i2str(m_iMission),1);
	IF_(m_iMission < 0);

	MissionBase* pMB = m_vMission[m_iMission].m_pInst;
	addMsg("Current mission: " + ((Kiss*)pMB->m_pKiss)->m_name,1);

	pMB->draw();
}

}
