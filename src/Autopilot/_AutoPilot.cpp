#include "_AutoPilot.h"

namespace kai
{

_AutoPilot::_AutoPilot()
{
	m_pAM = NULL;
	m_nAction = 0;
}

_AutoPilot::~_AutoPilot()
{
}

bool _AutoPilot::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	//create action instance
	Kiss* pCC = pK->o("action");
	IF_T(pCC->empty());
	Kiss** pItr = pCC->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Kiss* pAction = pItr[i];
		i++;

		bool bInst = false;
		F_INFO(pAction->v("bInst", &bInst));
		if (!bInst)continue;
		if (m_nAction >= N_ACTION)LOG(FATAL);

		ActionBase** pA = &m_pAction[m_nAction];
		m_nAction++;

		//Add action modules below

		ADD_ACTION(APcopter_base);
		ADD_ACTION(APcopter_DNNavoid);
		ADD_ACTION(APcopter_DNNnav);
		ADD_ACTION(APcopter_DNNodom);
		ADD_ACTION(APcopter_sensorAvoid);
		ADD_ACTION(APcopter_visualFollow);
		ADD_ACTION(APcopter_visualLanding);
		ADD_ACTION(APcopter_ZEDodom);

		ADD_ACTION(HM_base);
		ADD_ACTION(HM_kickBack);
		ADD_ACTION(HM_grass);
		ADD_ACTION(HM_rth);
		ADD_ACTION(HM_rth_approach);
		ADD_ACTION(HM_avoid);
		ADD_ACTION(HM_follow);

		ADD_ACTION(AProver_base);
		ADD_ACTION(AProver_follow);

#ifndef USE_OPENCV4TEGRA
		ADD_ACTION(RC_visualFollow);
#endif
		//Add action modules above

		m_nAction--;
		LOG_E("Unknown action class: "+pAction->m_class);
	}

	return true;
}

bool _AutoPilot::link(void)
{
	IF_F(!this->BASE::link());
	Kiss* pK = (Kiss*)m_pKiss;

	int i;
	for(i=0;i<m_nAction;i++)
	{
		ActionBase* pA = m_pAction[i];
		F_ERROR_F(pA->link());
	}

	string iName="";
	F_INFO(pK->v("_Automaton", &iName));
	m_pAM = (_Automaton*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _AutoPilot::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<<retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _AutoPilot::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		for(int i=0;i<m_nAction;i++)
		{
			m_pAction[i]->update();
		}

		this->autoFPSto();
	}

}

bool _AutoPilot::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	pWin->tabNext();
	for(int i=0;i<m_nAction;i++)
	{
		m_pAction[i]->draw();
	}
	pWin->tabPrev();

	return true;
}

void _AutoPilot::onMouse(MOUSE* pMouse)
{
	NULL_(pMouse);

	for(int i=0;i<m_nAction;i++)
	{
#ifndef USE_OPENCV4TEGRA
		if(*m_pAction[i]->getClass()=="RC_visualFollow")
		{
			((RC_visualFollow*)m_pAction[i])->onMouse(pMouse);
		}
#endif
	}
}


}
