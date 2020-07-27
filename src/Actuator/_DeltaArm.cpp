#include "_DeltaArm.h"

#ifdef USE_DYNAMIXEL

namespace kai
{

_DeltaArm::_DeltaArm()
{
	m_oprMode = 3;
	m_bGripper = false;

//	m_vPosRangeX.x = 0;
//	m_vPosRangeX.y = 100.0;
//	m_vPosRangeY.x = 0;
//	m_vPosRangeY.y = 100.0;
//	m_vPosRangeZ.x = 0;
//	m_vPosRangeZ.y = 100.0;
}

_DeltaArm::~_DeltaArm()
{
	//m_rdr.RobotTorqueOFF();
}

bool _DeltaArm::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("oprMode",&m_oprMode);
	pK->v("bGripper",&m_bGripper);

	//Initialize the robot and passing 3 as operation mode (Position control) as default
	string port;
	int baudRate = 57600;
	pK->v("port",&port);
	pK->v("baudRate",&baudRate);
	m_dr.RobotInit(m_oprMode, port.c_str(), baudRate);

	sleep(3);

	m_dr.RobotTorqueON();
	m_dr.GoHome();

	if(m_bGripper)
	{
		m_dr.GripperTorqueON();
		m_dr.GripperCheck();
		m_dr.GripperClose();
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

		readStatus();
		updatePos();

		this->autoFPSto();
	}
}

void _DeltaArm::readStatus(void)
{
	static uint64_t tLastStatus = 0;
	IF_(m_tStamp - tLastStatus < 100000);
	tLastStatus = m_tStamp;

	float v[3];
	m_dr.GetXYZ(v);
	m_vAxis[0].setRawP(v[0]);
	m_vAxis[1].setRawP(v[1]);
	m_vAxis[2].setRawP(v[2]);

	m_dr.GetRobotAngle(v);
	m_vAxis[3].setRawP(v[0]);
	m_vAxis[4].setRawP(v[1]);
	m_vAxis[5].setRawP(v[2]);
}

void _DeltaArm::updatePos(void)
{
	vFloat3 vP;
	vP.x = m_vAxis[0].getRawPtarget();
	vP.y = m_vAxis[1].getRawPtarget();
	vP.z = m_vAxis[2].getRawPtarget();
	m_dr.GotoPoint(vP.x, vP.y, vP.z);

	if(m_bGripper && m_vAxis[6].getPtarget() >= 0.0)
	{
		if(m_vAxis[6].getPtarget() > 0.5)
			m_dr.GripperOpen();
		else
			m_dr.GripperClose();
	}

}

void _DeltaArm::draw(void)
{
	this->_ActuatorBase::draw();

}

}

#endif
