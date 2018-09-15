#include "_AutoPilot.h"

namespace kai
{

_AutoPilot::_AutoPilot()
{
	m_pAM = NULL;
}

_AutoPilot::~_AutoPilot()
{
	for(int i=0; i<m_vAction.size(); i++)
	{
		DEL(m_vAction[i]);
	}

	m_vAction.clear();
}

bool _AutoPilot::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

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
		pAction->v("bInst", &bInst);
		IF_CONT(!bInst);

		ActionBase* pA = NULL;

		//Add action modules below

		ADD_ACTION(APcopter_arucoFollow);
		ADD_ACTION(APcopter_arucoLanding);
		ADD_ACTION(APcopter_base);
		ADD_ACTION(APcopter_DNNavoid);
		ADD_ACTION(APcopter_DNNnav);
		ADD_ACTION(APcopter_depthVision);
		ADD_ACTION(APcopter_distLidar);
		ADD_ACTION(APcopter_DNNfollow);
		ADD_ACTION(APcopter_DNNlanding);
		ADD_ACTION(APcopter_edgeHold);
		ADD_ACTION(APcopter_posCtrlRC);
		ADD_ACTION(APcopter_mission);

		ADD_ACTION(AProver_base);

		ADD_ACTION(HM_base);
		ADD_ACTION(HM_kickBack);
		ADD_ACTION(HM_grass);
		ADD_ACTION(HM_rth);
		ADD_ACTION(HM_rth_approach);
		ADD_ACTION(HM_avoid);
		ADD_ACTION(HM_marker);
		ADD_ACTION(HM_follow);

		ADD_ACTION(Traffic_base);
		ADD_ACTION(Traffic_speed);
		ADD_ACTION(Traffic_alert);

		ADD_ACTION(VEK_base);
		ADD_ACTION(VEK_avoid);
		ADD_ACTION(VEK_follow);

#ifdef USE_OPENCV_CONTRIB
		ADD_ACTION(RC_base);
		ADD_ACTION(RC_visualFollow);
#endif

		//Add action modules above

		IF_Fl(!pA, "Unknown action class: "+pAction->m_class);

		pAction->m_pInst = pA;
		m_vAction.push_back(pA);
	}

	string iName="";
	F_INFO(pK->v("_Automaton", &iName));
	m_pAM = (_Automaton*) (pK->root()->getChildInst(iName));

	return true;
}

bool _AutoPilot::start(void)
{
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

		for(int i=0; i<m_vAction.size(); i++)
		{
			m_vAction[i]->update();
		}

		this->autoFPSto();
	}
}

bool _AutoPilot::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	pWin->tabNext();
	for(int i=0; i<m_vAction.size(); i++)
	{
		m_vAction[i]->draw();
	}
	pWin->tabPrev();

	return true;
}

bool _AutoPilot::cli(int& iY)
{
	IF_F(!this->_ThreadBase::cli(iY));

	for(int i=0; i<m_vAction.size(); i++)
	{
		iY++;
		m_vAction[i]->cli(iY);
	}

	return true;
}

}
