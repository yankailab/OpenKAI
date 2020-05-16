#include "_LabArm.h"

#ifdef USE_DYNAMIXEL

namespace kai
{

_LabArm::_LabArm()
{
	m_oprMode = 3;
	m_bGripper = false;
	m_vPosRangeX.x = 0;
	m_vPosRangeX.y = 100.0;
	m_vPosRangeY.x = 0;
	m_vPosRangeY.y = 100.0;
	m_vPosRangeZ.x = 0;
	m_vPosRangeZ.y = 100.0;
}

_LabArm::~_LabArm()
{
	//m_la.RobotTorqueOFF();
}

bool _LabArm::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("oprMode",&m_oprMode);
	pK->v("bGripper",&m_bGripper);
	pK->v("vPosRangeX",&m_vPosRangeX);
	pK->v("vPosRangeY",&m_vPosRangeY);
	pK->v("vPosRangeZ",&m_vPosRangeZ);

	//Initialize the robot and passing 3 as operation mode (Position control) as default
	string port = "/dev/ttyUSB0";
	int baudRate = 57600;
	pK->v("port",&port);
	pK->v("baudRate",&baudRate);
	IF_Fl(!m_la.init(port.c_str(), baudRate), "LabArm init failed");

	m_la.MotorsInit(m_oprMode);

	m_la.TorqueON();
	m_la.GoHome();

	if(m_bGripper)
	{
		m_la.GripperON();
		m_la.GripperOpen();
	}

	//Moving the robot with the Awake-Standby-Home functions:
//	printf("\nArm awaking: \n");
//	m_la.Awake();
//	usleep(1000000);
//
//	printf("\nArm go to Standby position: \n");
//	m_la.StandBy();
//	usleep(1000000);
//
//	printf("\nArm go to Home position\n");
//	m_la.GoHome();
//	usleep(1000000);
//
//	//Moving the robot with XYZ coordonate: create a gripper postion table {X, Y, Z, rot_X, rot_Y, rot_Z} and run GotoXYZ.
//	float wantedposition[6] = {0, 340, 282, 10, 30, 0};
//	m_la.GotoXYZ(wantedposition);
//	usleep(2000000);
//
//	//Closing the gripper.
//	m_la.GripperClose();
//	usleep(2000000);
//
//	//Going back home and disactivate the torque
//	m_la.GripperOpen();
//	m_la.GoHome();
//	m_la.TorqueOFF();
//	m_la.GripperOFF();
//
//	//As we are using the motorMX430.h, we can also access motors function as follow:
//	m_la.motor1.PrintOperatingMode();
//	m_la.gripper.PrintOperatingMode();
//
//	//Joystick control Mode:
//	m_la.JoystickControl();

	return true;
}

bool _LabArm::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _LabArm::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		readStatus();
		updatePos();

		this->autoFPSto();
	}
}

void _LabArm::readStatus(void)
{
	static uint64_t tLastStatus = 0;
	IF_(m_tStamp - tLastStatus < 100000);
	tLastStatus = m_tStamp;

	float pP[6];
	m_la.GetXYZ(pP);
	m_vPos.x = pP[0];
	m_vPos.y = pP[1];
	m_vPos.z = pP[2];

	m_vNormPos.x = (float)(m_vPos.x - m_vPosRangeX.x) / (float)m_vPosRangeX.len();
	m_vNormPos.y = (float)(m_vPos.y - m_vPosRangeY.x) / (float)m_vPosRangeY.len();
	m_vNormPos.z = (float)(m_vPos.z - m_vPosRangeZ.x) / (float)m_vPosRangeZ.len();
}

void _LabArm::updatePos(void)
{
	vFloat3 vP;
	vP.x = m_vNormTargetPos.x * m_vPosRangeX.d() + m_vPosRangeX.x;
	vP.y = m_vNormTargetPos.y * m_vPosRangeY.d() + m_vPosRangeY.x;
	vP.z = m_vNormTargetPos.z * m_vPosRangeZ.d() + m_vPosRangeZ.x;

	//Moving the robot with XYZ coordonate: create a gripper postion table {X, Y, Z, rot_X, rot_Y, rot_Z} and run GotoXYZ.
//	float wantedposition[6] = {0, 340, 282, 10, 30, 0};
	float pTargetP[6] = {vP.x, vP.y, vP.z, 0, 0, 0};
	m_la.GotoXYZ(pTargetP);

	if(m_bGripper && m_vNormTargetPos.w >= 0.0)
	{
		if(m_vNormTargetPos.w > 0.5)
			m_la.GripperOpen();
		else
			m_la.GripperClose();
	}
}

void _LabArm::draw(void)
{
	this->_ActuatorBase::draw();
}

}
#endif
