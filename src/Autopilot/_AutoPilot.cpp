#include "_AutoPilot.h"

namespace kai
{

_AutoPilot::_AutoPilot()
{
	m_pAM = NULL;
	m_pRC = NULL;
	m_pMavlink = NULL;

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

}

_AutoPilot::~_AutoPilot()
{
}

bool _AutoPilot::init(Config* pConfig)
{
	if (this->_ThreadBase::init(pConfig)==false)
		return false;

	//link instance
	string iName = "";
	F_ERROR_F(pConfig->v("_Mavlink",&iName));
	m_pMavlink = (_Mavlink*)(pConfig->root()->getChildInstByName(&iName));
	F_ERROR_F(pConfig->v("_RC",&iName));
	m_pRC = (_RC*)(pConfig->root()->getChildInstByName(&iName));
	F_ERROR_F(pConfig->v("_Automaton",&iName));
	m_pAM = (_Automaton*)(pConfig->root()->getChildInstByName(&iName));


	CONTROL_PID cPID;
	RC_CHANNEL RC;
	int i;
	Config* pCC;

	pCC = pConfig->o("roll");
	if(pCC->empty())return false;

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
	if(pCC->empty())return false;

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
	if(pCC->empty())return false;

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
	if(pCC->empty())return false;

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

	pCC = pConfig->o("visualFollow");
	if(pCC->empty())return false;

	//For visual position locking
	F_ERROR_F(pCC->v("targetX", &m_ctrl.m_roll.m_targetPos));
	F_ERROR_F(pCC->v("targetY", &m_ctrl.m_pitch.m_targetPos));

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	//TODO: Create action classes

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
	if (m_pMavlink == NULL)
		return;

	//Sending Heartbeat at 1Hz
	uint64_t timeNow = get_time_usec();
	if (timeNow - m_lastHeartbeat >= USEC_1SEC)
	{
		m_pMavlink->sendHeartbeat();
		m_lastHeartbeat = timeNow;

#ifdef MAVLINK_DEBUG
		printf("   SENT HEARTBEAT:%d\n", (++m_iHeartbeat));
#endif
	}
}

bool _AutoPilot::draw(Frame* pFrame, iVec4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "AutoPilot FPS: " + i2str(getFrameRate()),
			cv::Point(pTextPos->m_x, pTextPos->m_y), FONT_HERSHEY_SIMPLEX, 0.5,
			Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

}
