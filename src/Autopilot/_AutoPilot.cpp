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
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	//create action instance
	Kiss* pCC = pK->o("action");
	CHECK_T(pCC->empty());
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

		ADD_ACTION(RC_visualFollow);
		ADD_ACTION(APMcopter_landing);
		ADD_ACTION(HM_base);
		ADD_ACTION(HM_follow);
		ADD_ACTION(APMrover_base);
		ADD_ACTION(APMrover_follow);

		LOG(INFO)<<"Unknown action class"+pAction->m_class;
	}

	return true;
}

bool _AutoPilot::link(void)
{
	NULL_F(m_pKiss);
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

bool _AutoPilot::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);

	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "AutoPilot FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	for(int i=0;i<m_nAction;i++)
	{
		m_pAction[i]->draw(pFrame, pTextPos);
	}

	return true;
}

void _AutoPilot::onMouse(MOUSE* pMouse)
{
	NULL_(pMouse);

	for(int i=0;i<m_nAction;i++)
	{
		if(*m_pAction[i]->getClass()=="RC_visualFollow")
		{
			((RC_visualFollow*)m_pAction[i])->onMouse(pMouse);
		}
	}
}


}
