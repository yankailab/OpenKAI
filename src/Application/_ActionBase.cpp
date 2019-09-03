#include "_ActionBase.h"

namespace kai
{

_ActionBase::_ActionBase()
{
	m_pMC = NULL;
	m_tStamp = 0;
	m_dTime = 0;
	m_iLastMission = 0;
	m_bMissionChanged = false;
	m_iPriority = 0;
}

_ActionBase::~_ActionBase()
{
}

bool _ActionBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("iPriority",&m_iPriority);

	string iName="";
	F_INFO(pK->v("_MissionControl", &iName));
	m_pMC = (_MissionControl*) (pK->root()->getChildInst(iName));
	NULL_T(m_pMC);

	string pAS[N_ACTIVEMISSION];
	int nAS = pK->a("activeMission", pAS, N_ACTIVEMISSION);
	for(int i=0; i<nAS; i++)
	{
		int iMission = m_pMC->getMissionIdx(pAS[i]);
		if(iMission<0)continue;

		m_vActiveMission.push_back(iMission);
	}

	m_iLastMission = m_pMC->getCurrentMissionIdx();

	return true;
}

int _ActionBase::check(void)
{
	return 0;
}

void _ActionBase::update(void)
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

bool _ActionBase::bActive(void)
{
	NULL_T(m_pMC);

	int iMission = m_pMC->getCurrentMissionIdx();
	for (int i : m_vActiveMission)
	{
		if(iMission == i)return true;
	}

	return false;
}

bool _ActionBase::bMissionChanged(void)
{
	return m_bMissionChanged;
}

bool _ActionBase::draw(void)
{
	return this->_ThreadBase::draw();
}

}
