#include "_AutopilotBase.h"

namespace kai
{

_AutopilotBase::_AutopilotBase()
{
	m_pMC = NULL;
	m_iLastMission = 0;
	m_bMissionChanged = false;
	m_tStamp = 0;
	m_dTime = 0;
	m_pCtrl = NULL;
}

_AutopilotBase::~_AutopilotBase()
{
}

bool _AutopilotBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName="";
	F_INFO(pK->v("_MissionControl", &iName));
	m_pMC = (_MissionControl*) (pK->root()->getChildInst(iName));
	NULL_T(m_pMC);

	vector<string> vAS;
	pK->a("activeMission", &vAS);
	for(int i=0; i<vAS.size(); i++)
	{
		int iMission = m_pMC->getMissionIdx(vAS[i]);
		if(iMission<0)continue;

		m_vActiveMission.push_back(iMission);
	}

	m_iLastMission = m_pMC->getCurrentMissionIdx();

	return true;
}

int _AutopilotBase::check(void)
{
	return 0;
}

void _AutopilotBase::update(void)
{
	uint64_t newTime = getTimeUsec();
	m_dTime = newTime - m_tStamp;
	m_tStamp = newTime;

	NULL_(m_pMC);
	int currentMission = m_pMC->getCurrentMissionIdx();
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

bool _AutopilotBase::bActive(void)
{
	NULL_T(m_pMC);

	int iMission = m_pMC->getCurrentMissionIdx();
	for (int i : m_vActiveMission)
	{
		if(iMission == i)return true;
	}

	return false;
}

bool _AutopilotBase::bMissionChanged(void)
{
	return m_bMissionChanged;
}

void _AutopilotBase::draw(void)
{
	this->_ThreadBase::draw();
}

}
