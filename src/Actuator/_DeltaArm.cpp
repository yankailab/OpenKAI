#include "_DeltaArm.h"

#ifdef USE_DYNAMIXEL

namespace kai
{

_DeltaArm::_DeltaArm()
{
}

_DeltaArm::~_DeltaArm()
{
	//DRC.RobotTorqueOFF();
}

bool _DeltaArm::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	//Initialize the robot and passing 3 as operation mode (Position control) as default
	m_rdr.RobotInit(3);
	m_rdr.RobotTorqueON();
	m_rdr.GripperTorqueON();
	m_rdr.GoHome();
	m_rdr.GripperCheck();

	float RobotAng[3];
	m_rdr.GetRobotAngle(RobotAng);
	printf("RobotAng1 %f \n", RobotAng[0]);
	printf("RobotAng2 %f \n", RobotAng[1]);
	printf("RobotAng3 %f \n", RobotAng[2]);

	float XYZ[3];
	m_rdr.GetXYZ(XYZ);
	printf("X %f \n", XYZ[0]);
	printf("Y %f \n", XYZ[1]);
	printf("Z %f \n", XYZ[2]);

	while(1)
	{
		m_rdr.GotoPoint(-500,0,-700);
		m_rdr.GripperClose();
		m_rdr.GripperOpen();

	}

	return true;
}

bool _DeltaArm::start(void)
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

void _DeltaArm::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

//		updateMotion();
//
//		while(readCMD())
//		{
//			handleCMD();
//			m_nCMDrecv++;
//		}

		this->autoFPSto();
	}
}

bool _DeltaArm::draw(void)
{
	IF_F(!this->_ActuatorBase::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;

	return true;
}

bool _DeltaArm::console(int& iY)
{
	IF_F(!this->_ActuatorBase::console(iY));
	string msg;

	return true;
}

}

#endif
