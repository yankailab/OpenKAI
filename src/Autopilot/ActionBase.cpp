#include "ActionBase.h"

namespace kai
{

ActionBase::ActionBase()
{
	m_pAM = NULL;
	m_tStamp = 0;
	m_dTime = 0;
	m_iLastMission = 0;
	m_bMissionChanged = false;
	m_iPriority = 0;
}

ActionBase::~ActionBase()
{
}

bool ActionBase::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, iPriority);

	//link
	string iName="";
	F_INFO(pK->v("_MissionControl", &iName));
	m_pAM = (_MissionControl*) (pK->root()->getChildInst(iName));
	NULL_T(m_pAM);

	string pAS[N_ACTIVEMISSION];
	int nAS = pK->array("activeState", pAS, N_ACTIVEMISSION);
	for(int i=0; i<nAS; i++)
	{
		int iMission = m_pAM->getMissionIdx(pAS[i]);
		if(iMission<0)continue;

		m_vActiveMission.push_back(iMission);
	}

	m_iLastMission = m_pAM->getCurrentMissionIdx();

	return true;
}

int ActionBase::check(void)
{
	NULL__(m_pAM,-1);

	return 0;
}

void ActionBase::update(void)
{
	uint64_t newTime = getTimeUsec();
	m_dTime = newTime - m_tStamp;
	m_tStamp = newTime;

	NULL_(m_pAM);
	int currentMission = m_pAM->getCurrentMissionIdx();
	if(m_iLastMission != currentMission)
	{
		m_bMissionChanged = true;
		m_iLastMission = currentMission;
	}
	else
	{
		m_bMissionChanged = false;
	}
}

bool ActionBase::isActive(void)
{
	NULL_F(m_pAM);

	int iMission = m_pAM->getCurrentMissionIdx();
	for (int i : m_vActiveMission)
	{
		if(iMission == i)return true;
	}

	return false;
}

bool ActionBase::isStateChanged(void)
{
	return m_bMissionChanged;
}

bool ActionBase::draw(void)
{
	return this->BASE::draw();
}

}
