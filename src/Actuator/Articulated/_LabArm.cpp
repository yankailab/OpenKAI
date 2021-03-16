#include "_LabArm.h"

#ifdef USE_DYNAMIXEL

namespace kai
{

_LabArm::_LabArm()
{
	m_oprMode = 3;
	m_bGripper = false;

//	m_vPosRangeX.init(0.0, 100.0);
//	m_vPosRangeY.init(0.0, 100.0);
//	m_vPosRangeZ.init(0.0, 100.0);
//
//	m_vRotRangeX.init(0.0, 180.0);
//	m_vRotRangeY.init(0.0, 180.0);
//	m_vRotRangeZ.init(0.0, 180.0);
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
    NULL_F(m_pT);
    return m_pT->start(getUpdate, this);
}

void _LabArm::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		readStatus();
		updatePos();

		m_pT->autoFPSto();
	}
}

void _LabArm::readStatus(void)
{
	static uint64_t tLastStatus = 0;
	IF_(m_tStamp - tLastStatus < 100000);
	tLastStatus = m_pT->getTfrom();

	float v[6];
	m_la.GetXYZ(v);
	m_vAxis[0].setRawP(v[0]);
	m_vAxis[1].setRawP(v[1]);
	m_vAxis[2].setRawP(v[2]);
	m_vAxis[3].setRawP(v[3]);
	m_vAxis[4].setRawP(v[4]);
	m_vAxis[5].setRawP(v[5]);
}

void _LabArm::updatePos(void)
{
	vFloat3 vP;
	vP.x = m_vAxis[0].getRawPtarget();
	vP.y = m_vAxis[1].getRawPtarget();
	vP.z = m_vAxis[2].getRawPtarget();

	vFloat3 vA;
	vA.x = m_vAxis[3].getRawPtarget();
	vA.y = m_vAxis[4].getRawPtarget();
	vA.z = m_vAxis[5].getRawPtarget();

	float pTargetP[6] = {vP.x, vP.y, vP.z, 0,0,0};//vR.x, vR.y, vR.z};
	m_la.GotoXYZ(pTargetP);

//	m_pT->sleepTime(USEC_1SEC*3);
//
//	m_la.motor3.Goto(vR.x);
//	m_la.motor5.Goto(vR.y);
//	m_la.motor6.Goto(vR.z);
//
//	m_pT->sleepTime(USEC_1SEC*3);

	if(m_bGripper && m_vAxis[6].getPtarget() >= 0.0)
	{
		if(m_vAxis[6].getPtarget() > 0.5)
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
