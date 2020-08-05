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
	m_iM = 0;
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

	Kiss* pCC = pK->child("mission");
	IF_T(pCC->empty());

	MISSION_INST M;
	int i = 0;
	while (1)
	{
		Kiss* pKM = pCC->child(i++);
		if(pKM->empty())break;

		M.init();

		//Add action modules below

		ADD_MISSION(Mission);
		ADD_MISSION(Takeoff);
		ADD_MISSION(Loiter);
		ADD_MISSION(Waypoint);
		ADD_MISSION(Goto);
		ADD_MISSION(RTH);
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
	m_iM = getMissionIdx(startMission);
	if(m_iM < 0)
		m_iM = 0;

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

		Mission* pMission = m_vMission[m_iM].m_pInst;
		if(pMission->update())
		{
			transit(pMission->m_nextMission);
		}

		this->autoFPSto();
	}
}

void _MissionControl::transit(void)
{
	Mission* pM = m_vMission[m_iM].m_pInst;
	NULL_(pM);

	transit(pM->m_nextMission);
}

void _MissionControl::transit(const string& mName)
{
	int iNext = getMissionIdx(mName);
	transit(iNext);
}

void _MissionControl::transit(int iM)
{
	IF_(iM < 0);
	IF_(iM >= m_vMission.size());
	IF_(iM == m_iM);

	Mission* pMission = m_vMission[m_iM].m_pInst;
	pMission->reset();

	m_iM = iM;
}

int _MissionControl::getMissionIdx(const string& mName)
{
	for(unsigned int i=0; i<m_vMission.size(); i++)
	{
		if(((Kiss*)m_vMission[i].m_pInst->m_pKiss)->m_name == mName)
			return i;
	}

	return -1;
}

Mission* _MissionControl::getMission(void)
{
	IF_N(m_iM >= m_vMission.size());
	IF_N(m_iM < 0);
	return m_vMission[m_iM].m_pInst;
}

int _MissionControl::getMissionIdx(void)
{
	return m_iM;
}

string _MissionControl::getMissionName(void)
{
	IF_N(m_iM >= m_vMission.size());
	IF_N(m_iM < 0);

	string name = ((Kiss*)m_vMission[m_iM].m_pInst->m_pKiss)->m_name;
	return name;
}

MISSION_TYPE _MissionControl::getMissionType(void)
{
	Mission* pMB = getMission();

	if(!pMB)
		return mission_unknown;

	return pMB->m_type;
}

void _MissionControl::draw(void)
{
	this->_ThreadBase::draw();

	addMsg("nMission: "+i2str(m_vMission.size()),1);
	IF_(m_vMission.size() <= 0);

	addMsg("iMission: "+i2str(m_iM),1);
	IF_(m_iM < 0);

	Mission* pMB = m_vMission[m_iM].m_pInst;
	addMsg("Current mission: " + ((Kiss*)pMB->m_pKiss)->m_name,1);

	if(pMB->type() == mission_wp)
		((Waypoint*)pMB)->draw();
	else
		pMB->draw();

}

}
