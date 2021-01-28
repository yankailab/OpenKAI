#include "_MissionBase.h"

namespace kai
{

_MissionBase::_MissionBase()
{
	m_pMC = NULL;
	m_iLastMission = 0;
	m_bMissionChanged = false;
}

_MissionBase::~_MissionBase()
{
}

bool _MissionBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName="";
	pK->v("_MissionControl", &iName);
	m_pMC = (_MissionControl*) (pK->getInst(iName));
	NULL_T(m_pMC);

	vector<string> vAS;
	pK->a("activeMission", &vAS);
	for(int i=0; i<vAS.size(); i++)
	{
		int iMission = m_pMC->getMissionIdxByName (vAS[i]);
		if(iMission<0)continue;

		m_vActiveMission.push_back(iMission);
	}

	m_iLastMission = m_pMC->getMissionIdx();

	return true;
}

int _MissionBase::check(void)
{
	return 0;
}

void _MissionBase::update(void)
{
	NULL_(m_pMC);
	int currentMission = m_pMC->getMissionIdx();
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

bool _MissionBase::bActive(void)
{
	NULL_T(m_pMC);
	IF_T(m_vActiveMission.empty());

	int iMission = m_pMC->getMissionIdx();
	for (int i : m_vActiveMission)
	{
		if(iMission == i)return true;
	}

	return false;
}

bool _MissionBase::bMissionChanged(void)
{
	return m_bMissionChanged;
}

void _MissionBase::drawActive(void)
{
	if(!bActive())
		addMsg("[Inactive]",1);
	else
		addMsg("[Active]",1);
}

void _MissionBase::draw(void)
{
	this->_ThreadBase::draw();
}

}
