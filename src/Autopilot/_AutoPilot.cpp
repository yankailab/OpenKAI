#include "_AutoPilot.h"

namespace kai
{

_AutoPilot::_AutoPilot()
{
	m_pAM = NULL;
	m_nAction = 0;
	m_pAA = NULL;

	m_ctrl.reset();
}

_AutoPilot::~_AutoPilot()
{
}

bool _AutoPilot::init(Config* pConfig)
{
	CHECK_F(this->_ThreadBase::init(pConfig) == false);

	//link instance
	string iName = "";
	F_INFO(pConfig->v("_Mavlink", &iName));
	m_ctrl.m_pMavlink = (_Mavlink*) (pConfig->root()->getChildInstByName(&iName));
	F_INFO(pConfig->v("_RC", &iName));
	m_ctrl.m_pRC = (_RC*) (pConfig->root()->getChildInstByName(&iName));
	F_INFO(pConfig->v("_Automaton", &iName));
	m_pAM = (_Automaton*) (pConfig->root()->getChildInstByName(&iName));

	CONTROL_PID cPID;
	RC_CHANNEL RC;
	int i;
	Config* pCC;

	pCC = pConfig->o("roll");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	F_ERROR_F(pCC->v("pwmL", &RC.m_pwmL));
	F_ERROR_F(pCC->v("pwmH", &RC.m_pwmH));
	F_ERROR_F(pCC->v("pwmN", &RC.m_pwmN));
	F_ERROR_F(pCC->v("pwmCh", &RC.m_iCh));

	m_ctrl.m_roll.m_pid = cPID;
	m_ctrl.m_roll.m_RC = RC;

	pCC = pConfig->o("pitch");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	F_ERROR_F(pCC->v("pwmL", &RC.m_pwmL));
	F_ERROR_F(pCC->v("pwmH", &RC.m_pwmH));
	F_ERROR_F(pCC->v("pwmN", &RC.m_pwmN));
	F_ERROR_F(pCC->v("pwmCh", &RC.m_iCh));

	m_ctrl.m_pitch.m_pid = cPID;
	m_ctrl.m_pitch.m_RC = RC;

	pCC = pConfig->o("alt");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	F_ERROR_F(pCC->v("pwmL", &RC.m_pwmL));
	F_ERROR_F(pCC->v("pwmH", &RC.m_pwmH));
	F_ERROR_F(pCC->v("pwmN", &RC.m_pwmN));
	F_ERROR_F(pCC->v("pwmCh", &RC.m_iCh));

	m_ctrl.m_alt.m_pid = cPID;
	m_ctrl.m_alt.m_RC = RC;

	pCC = pConfig->o("yaw");
	CHECK_F(pCC->empty());

	F_ERROR_F(pCC->v("P", &cPID.m_P));
	F_ERROR_F(pCC->v("I", &cPID.m_I));
	F_ERROR_F(pCC->v("Imax", &cPID.m_Imax));
	F_ERROR_F(pCC->v("D", &cPID.m_D));
	F_ERROR_F(pCC->v("dT", &cPID.m_dT));
	F_ERROR_F(pCC->v("pwmL", &RC.m_pwmL));
	F_ERROR_F(pCC->v("pwmH", &RC.m_pwmH));
	F_ERROR_F(pCC->v("pwmN", &RC.m_pwmN));
	F_ERROR_F(pCC->v("pwmCh", &RC.m_iCh));

	m_ctrl.m_yaw.m_pid = cPID;
	m_ctrl.m_yaw.m_RC = RC;

	//create action instance
	pCC = pConfig->o("action");
	CHECK_T(pCC->empty());

	Config** pItr = pCC->getChildItr();

	i = 0;
	while (pItr[i])
	{
		Config* pAction = pItr[i];
		i++;

		bool bInst = false;
		F_INFO(pAction->v("bInst", &bInst));
		if (!bInst)
			continue;

		if (m_nAction >= N_ACTION)
			LOG(FATAL);
		ActionBase** pA = &m_pAction[m_nAction];
		m_nAction++;

		if (pAction->m_class == "VisualFollow")
		{
			*pA = new VisualFollow();
			F_ERROR_F(((VisualFollow* )(*pA))->VisualFollow::init(pAction, &m_ctrl));
			m_pAA = *pA;
		}
		else if (pAction->m_class == "Landing")
		{
			*pA = new Landing();
			F_ERROR_F(((Landing* )(*pA))->Landing::init(pAction, &m_ctrl));
		}
		else
		{
			LOG(INFO)<<"Unknown action class";
		}

	}

	//init controls
	m_ctrl.m_lastHeartbeat = 0;
	m_ctrl.m_iHeartbeat = 0;

	return true;
}

bool _AutoPilot::start(void)
{
	//Start thread
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode << " in _State::start().pthread_create()";
		m_bThreadON = false;
		return false;
	}

	LOG(INFO)<< "AutoPilot.start()";

	return true;
}

void _AutoPilot::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		//TODO:Execute actions based on Automaton State

		sendHeartbeat();

		this->autoFPSto();
	}

}

void _AutoPilot::sendHeartbeat(void)
{
	NULL_(m_ctrl.m_pMavlink);

	//Sending Heartbeat at 1Hz
	uint64_t timeNow = get_time_usec();
	if (timeNow - m_ctrl.m_lastHeartbeat >= USEC_1SEC)
	{
		m_ctrl.m_pMavlink->sendHeartbeat();
		m_ctrl.m_lastHeartbeat = timeNow;

#ifdef MAVLINK_DEBUG
		printf("   SENT HEARTBEAT:%d\n", (++m_ctrl.m_iHeartbeat));
#endif
	}
}

bool _AutoPilot::draw(Frame* pFrame, iVec4* pTextPos)
{
	NULL_F(pFrame);

	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "AutoPilot FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	CHECK_T(!m_pAA);

	string* className = m_pAA->getClass();
	if(*className == "VisualFollow")
	{
		((VisualFollow*)m_pAA)->draw(pFrame, pTextPos);
	}

	return true;
}

void _AutoPilot::onMouse(MOUSE* pMouse)
{
	NULL_(m_pAA);
	NULL_(pMouse);
	CHECK_(*(m_pAA->getClass())!="VisualFollow");

	((VisualFollow*)m_pAA)->onMouse(pMouse);
}


}
