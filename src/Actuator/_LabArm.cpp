#include "_LabArm.h"

#ifdef USE_DYNAMIXEL

namespace kai
{

_LabArm::_LabArm()
{
	m_oprMode = 3;
	m_bGripper = false;

	m_vPosRangeX.init(0.0, 100.0);
	m_vPosRangeY.init(0.0, 100.0);
	m_vPosRangeZ.init(0.0, 100.0);

	m_vRotRangeX.init(0.0, 180.0);
	m_vRotRangeY.init(0.0, 180.0);
	m_vRotRangeZ.init(0.0, 180.0);
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

	pK->v("vRotRangeX",&m_vRotRangeX);
	pK->v("vRotRangeY",&m_vRotRangeY);
	pK->v("vRotRangeZ",&m_vRotRangeZ);

	//Initialize the robot and passing 3 as operation mode (Position control) as default
	string port = "/dev/ttyUSB0";
	int baudRate = 57600;
	pK->v("port",&port);
	pK->v("baudRate",&baudRate);
	IF_Fl(!m_la.init(port.c_str(), baudRate), "LabArm init failed");

	m_la.MotorsInit(m_oprMode);
	m_la.TorqueON();

	if(m_bGripper)
	{
		m_la.GripperON();
		m_la.GripperOpen();
	}

	m_la.GoHome();

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

	m_vRot.x = pP[3];
	m_vRot.y = pP[4];
	m_vRot.z = pP[5];
	m_vNormRot.x = (float)(m_vRot.x - m_vRotRangeX.x) / (float)m_vRotRangeX.len();
	m_vNormRot.y = (float)(m_vRot.y - m_vRotRangeY.x) / (float)m_vRotRangeY.len();
	m_vNormRot.z = (float)(m_vRot.z - m_vRotRangeZ.x) / (float)m_vRotRangeZ.len();

}

void _LabArm::updatePos(void)
{
	vFloat3 vP;
	vP.x = m_vNormTargetPos.x * m_vPosRangeX.d() + m_vPosRangeX.x;
	vP.y = m_vNormTargetPos.y * m_vPosRangeY.d() + m_vPosRangeY.x;
	vP.z = m_vNormTargetPos.z * m_vPosRangeZ.d() + m_vPosRangeZ.x;

	vFloat3 vR;
	vR.x = m_vNormTargetRot.x * m_vRotRangeX.d() + m_vRotRangeX.x;
	vR.y = m_vNormTargetRot.y * m_vRotRangeY.d() + m_vRotRangeY.x;
	vR.z = m_vNormTargetRot.z * m_vRotRangeZ.d() + m_vRotRangeZ.x;

	float pTargetP[6] = {vP.x, vP.y, vP.z, 0,0,0};//vR.x, vR.y, vR.z};
	m_la.GotoXYZ(pTargetP);

//	this->sleepTime(USEC_1SEC*3);
//
//	m_la.motor3.Goto(vR.x);
//	m_la.motor5.Goto(vR.y);
//	m_la.motor6.Goto(vR.z);
//
//	this->sleepTime(USEC_1SEC*3);

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
