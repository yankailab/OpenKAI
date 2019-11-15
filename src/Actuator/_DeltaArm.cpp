#include "_DeltaArm.h"

#ifdef USE_DYNAMIXEL

namespace kai
{

_DeltaArm::_DeltaArm()
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
	pK->v("vPosRangeX",&m_vPosRangeX);
	pK->v("vPosRangeY",&m_vPosRangeY);
	pK->v("vPosRangeZ",&m_vPosRangeZ);

	//Initialize the robot and passing 3 as operation mode (Position control) as default
	m_rdr.RobotInit(m_oprMode);

	sleep(3);

	m_rdr.RobotTorqueON();
	m_rdr.GoHome();

	if(m_bGripper)
	{
		m_rdr.GripperTorqueON();
		m_rdr.GripperCheck();
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
	m_rdr.GetRobotAngle(v);
	m_vAngle.x = v[0];
	m_vAngle.y = v[1];
	m_vAngle.z = v[2];

	m_rdr.GetXYZ(v);
	m_vPos.x = v[0];
	m_vPos.y = v[1];
	m_vPos.z = v[2];

	m_vNormPos.x = (float)(m_vPos.x - m_vPosRangeX.x) / (float)m_vPosRangeX.len();
	m_vNormPos.y = (float)(m_vPos.y - m_vPosRangeY.x) / (float)m_vPosRangeY.len();
	m_vNormPos.z = (float)(m_vPos.z - m_vPosRangeZ.x) / (float)m_vPosRangeZ.len();
}

void _DeltaArm::updatePos(void)
{
	vFloat3 vP;
	vP.x = m_vNormTargetPos.x * m_vPosRangeX.d() + m_vPosRangeX.x;
	vP.y = m_vNormTargetPos.y * m_vPosRangeY.d() + m_vPosRangeY.x;
	vP.z = m_vNormTargetPos.z * m_vPosRangeZ.d() + m_vPosRangeZ.x;
	m_rdr.GotoPoint(vP.x, vP.y, vP.z);

	if(m_bGripper && m_vNormTargetPos.w >= 0.0)
	{
		if(m_vNormTargetPos.w > 0.5)
			m_rdr.GripperOpen();
		else
			m_rdr.GripperClose();
	}

}

void _DeltaArm::draw(void)
{
	this->_ActuatorBase::draw();

}

}

#endif
