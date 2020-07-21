#include "_AProver_drive.h"

namespace kai
{

_AProver_drive::_AProver_drive()
{
	m_pAP = NULL;

	m_nSpeed = 0.0;
	m_kSpeed = 1.0;
	m_yaw = 0.0;

	m_speed = 1.0;	//1.0m/s
	m_yawMode = 1.0;

	m_bSetYawSpeed = true;
	m_bRcChanOverride = true;
	m_pwmM = 1500;
	m_pwmD = 500;
	m_pRcYaw = NULL;
	m_pRcThrottle = NULL;
}

_AProver_drive::~_AProver_drive()
{
}

bool _AProver_drive::init(void* pKiss)
{
	IF_F(!this->_MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nSpeed",&m_nSpeed);
	pK->v("kSpeed",&m_kSpeed);
	pK->v("yaw",&m_yaw);

	pK->v("speed",&m_speed);
	pK->v("yawMode",&m_yawMode);

	pK->v("bSetYawSpeed",&m_bSetYawSpeed);
	pK->v("bRcChanOverride",&m_bRcChanOverride);

	uint16_t* pRC[18];
	pRC[0] = &m_rcOverride.chan1_raw;
	pRC[1] = &m_rcOverride.chan2_raw;
	pRC[2] = &m_rcOverride.chan3_raw;
	pRC[3] = &m_rcOverride.chan4_raw;
	pRC[4] = &m_rcOverride.chan5_raw;
	pRC[5] = &m_rcOverride.chan6_raw;
	pRC[6] = &m_rcOverride.chan7_raw;
	pRC[7] = &m_rcOverride.chan8_raw;
	pRC[8] = &m_rcOverride.chan9_raw;
	pRC[9] = &m_rcOverride.chan10_raw;
	pRC[10] = &m_rcOverride.chan11_raw;
	pRC[11] = &m_rcOverride.chan12_raw;
	pRC[12] = &m_rcOverride.chan13_raw;
	pRC[13] = &m_rcOverride.chan14_raw;
	pRC[14] = &m_rcOverride.chan15_raw;
	pRC[15] = &m_rcOverride.chan16_raw;
	pRC[16] = &m_rcOverride.chan17_raw;
	pRC[17] = &m_rcOverride.chan18_raw;

	int iRcYaw = 2;
	pK->v("iRcYaw", &iRcYaw);
	int iRcThrottle = 3;
	pK->v("iRcThrottle", &iRcThrottle);

	IF_Fl(iRcYaw > 18, "RC yaw channel exceeds limit");
	IF_Fl(iRcThrottle > 18, "RC throttle channel exceeds limit");

	m_pRcYaw = pRC[iRcYaw];
	m_pRcThrottle = pRC[iRcThrottle];

	pK->v("pwmM", &m_pwmM);
	pK->v("pwmD", &m_pwmD);

	string iName;
	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->getInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

bool _AProver_drive::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: " << retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AProver_drive::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMav,-1);

	return this->_MissionBase::check();
}

void _AProver_drive::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateDrive();

		this->autoFPSto();
	}
}

bool _AProver_drive::updateDrive(void)
{
	IF_F(check() < 0);

	if(m_bSetYawSpeed)
	{
		m_pAP->m_pMav->clNavSetYawSpeed(m_yaw,
											m_nSpeed * m_kSpeed * m_speed,
											m_yawMode);
	}

	if(m_bRcChanOverride)
	{
		*m_pRcYaw = m_yaw * m_pwmD + m_pwmM;
		*m_pRcThrottle = m_nSpeed * m_pwmD + m_pwmM;
		m_pAP->m_pMav->rcChannelsOverride(m_rcOverride);
	}

	return true;
}

void _AProver_drive::setSpeed(float nSpeed)
{
	m_nSpeed = nSpeed;
}

void _AProver_drive::setYaw(float yaw)
{
	m_yaw = yaw;
}

void _AProver_drive::setYawMode(bool bRelative)
{
	if(bRelative)
		m_yawMode = 1.0;
	else
		m_yawMode = 0.0;
}

void _AProver_drive::draw(void)
{
	this->_MissionBase::draw();

	addMsg("speed=" + f2str(m_speed) + ", kSpeed=" + f2str(m_kSpeed) + ", nSpeed=" + f2str(m_nSpeed));
	addMsg("yawMode=" + f2str(m_yawMode) + ", yaw=" + f2str(m_yaw));
}

}
