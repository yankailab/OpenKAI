#include "_AutoPilot.h"

namespace kai
{

_AutoPilot::_AutoPilot()
{
	m_pAM = NULL;
	m_pVI = NULL;
	m_pMavlink = NULL;

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

}

_AutoPilot::~_AutoPilot()
{
}

bool _AutoPilot::init(Config* pConfig, string name)
{
	if(pConfig==NULL)return false;
	if(name.empty())return false;

	Config* pC = pConfig->o(name);
	if(pC->empty())return false;

	CONTROL_PID cPID;
	RC_CHANNEL RC;
	int i;
	Config* pCC;

	pCC = pC->o("roll");
	if(pCC->empty())return false;

	CHECK_ERROR(pCC->v("P", &cPID.m_P));
	CHECK_ERROR(pCC->v("I", &cPID.m_I));
	CHECK_ERROR(pCC->v("Imax", &cPID.m_Imax));
	CHECK_ERROR(pCC->v("D", &cPID.m_D));
	CHECK_ERROR(pCC->v("dT", &cPID.m_dT));
	CHECK_ERROR(pCC->v("pwmL", &RC.m_pwmL));
	CHECK_ERROR(pCC->v("pwmH", &RC.m_pwmH));
	CHECK_ERROR(pCC->v("pwmN", &RC.m_pwmN));
	CHECK_ERROR(pCC->v("pwmCh", &RC.m_iCh));

	m_roll.m_pid = cPID;
	m_roll.m_RC = RC;

	pCC = pC->o("pitch");
	if(pCC->empty())return false;

	CHECK_ERROR(pCC->v("P", &cPID.m_P));
	CHECK_ERROR(pCC->v("I", &cPID.m_I));
	CHECK_ERROR(pCC->v("Imax", &cPID.m_Imax));
	CHECK_ERROR(pCC->v("D", &cPID.m_D));
	CHECK_ERROR(pCC->v("dT", &cPID.m_dT));
	CHECK_ERROR(pCC->v("pwmL", &RC.m_pwmL));
	CHECK_ERROR(pCC->v("pwmH", &RC.m_pwmH));
	CHECK_ERROR(pCC->v("pwmN", &RC.m_pwmN));
	CHECK_ERROR(pCC->v("pwmCh", &RC.m_iCh));

	m_pitch.m_pid = cPID;
	m_pitch.m_RC = RC;

	pCC = pC->o("alt");
	if(pCC->empty())return false;

	CHECK_ERROR(pCC->v("P", &cPID.m_P));
	CHECK_ERROR(pCC->v("I", &cPID.m_I));
	CHECK_ERROR(pCC->v("Imax", &cPID.m_Imax));
	CHECK_ERROR(pCC->v("D", &cPID.m_D));
	CHECK_ERROR(pCC->v("dT", &cPID.m_dT));
	CHECK_ERROR(pCC->v("pwmL", &RC.m_pwmL));
	CHECK_ERROR(pCC->v("pwmH", &RC.m_pwmH));
	CHECK_ERROR(pCC->v("pwmN", &RC.m_pwmN));
	CHECK_ERROR(pCC->v("pwmCh", &RC.m_iCh));

	m_alt.m_pid = cPID;
	m_alt.m_RC = RC;

	pCC = pC->o("yaw");
	if(pCC->empty())return false;

	CHECK_ERROR(pCC->v("P", &cPID.m_P));
	CHECK_ERROR(pCC->v("I", &cPID.m_I));
	CHECK_ERROR(pCC->v("Imax", &cPID.m_Imax));
	CHECK_ERROR(pCC->v("D", &cPID.m_D));
	CHECK_ERROR(pCC->v("dT", &cPID.m_dT));
	CHECK_ERROR(pCC->v("pwmL", &RC.m_pwmL));
	CHECK_ERROR(pCC->v("pwmH", &RC.m_pwmH));
	CHECK_ERROR(pCC->v("pwmN", &RC.m_pwmN));
	CHECK_ERROR(pCC->v("pwmCh", &RC.m_iCh));

	m_yaw.m_pid = cPID;
	m_yaw.m_RC = RC;

	pCC = pC->o("visualFollow");
	if(pCC->empty())return false;

	//For visual position locking
	CHECK_ERROR(pCC->v("targetX", &m_roll.m_targetPos));
	CHECK_ERROR(pCC->v("targetY", &m_pitch.m_targetPos));


	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

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

//int* _AutoPilot::getPWMOutput(void)
//{
//	return m_RC;
//}

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
