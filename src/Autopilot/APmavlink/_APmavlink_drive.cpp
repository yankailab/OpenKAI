#include "_APmavlink_drive.h"

namespace kai
{

	_APmavlink_drive::_APmavlink_drive()
	{
		m_pAP = nullptr;

		m_yawMode = 1.0;
		m_bSetYawSpeed = false;
		m_bRcChanOverride = false;

		m_steer = 0.0;
		m_speed = 0.0;
		m_pwmM = 1500;
		m_pwmD = 500;
		m_pRcYaw = nullptr;
		m_pRcThrottle = nullptr;
	}

	_APmavlink_drive::~_APmavlink_drive()
	{
	}

	int _APmavlink_drive::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

		= j.value("bSetYawSpeed", &m_bSetYawSpeed);
		= j.value("yawMode", &m_yawMode);
		= j.value("bRcChanOverride", &m_bRcChanOverride);

		= j.value("steer", &m_steer);
		= j.value("speed", &m_speed);
		= j.value("pwmM", &m_pwmM);
		= j.value("pwmD", &m_pwmD);

		uint16_t *pRC[19];
		pRC[0] = NULL;
		pRC[1] = &m_rcOverride.chan1_raw;
		pRC[2] = &m_rcOverride.chan2_raw;
		pRC[3] = &m_rcOverride.chan3_raw;
		pRC[4] = &m_rcOverride.chan4_raw;
		pRC[5] = &m_rcOverride.chan5_raw;
		pRC[6] = &m_rcOverride.chan6_raw;
		pRC[7] = &m_rcOverride.chan7_raw;
		pRC[8] = &m_rcOverride.chan8_raw;
		pRC[9] = &m_rcOverride.chan9_raw;
		pRC[10] = &m_rcOverride.chan10_raw;
		pRC[11] = &m_rcOverride.chan11_raw;
		pRC[12] = &m_rcOverride.chan12_raw;
		pRC[13] = &m_rcOverride.chan13_raw;
		pRC[14] = &m_rcOverride.chan14_raw;
		pRC[15] = &m_rcOverride.chan15_raw;
		pRC[16] = &m_rcOverride.chan16_raw;
		pRC[17] = &m_rcOverride.chan17_raw;
		pRC[18] = &m_rcOverride.chan18_raw;

		for (int i = 1; i < 19; i++)
			*pRC[i] = UINT16_MAX;

		int iRcYaw = 1;
		= j.value("iRcYaw", &iRcYaw);
		IF__(iRcYaw <= 0 || iRcYaw > 18, OK_ERR_INVALID_VALUE);
		m_pRcYaw = pRC[iRcYaw];

		int iRcThrottle = 3;
		= j.value("iRcThrottle", &iRcThrottle);
		IF__(iRcThrottle <= 0 || iRcThrottle > 18, OK_ERR_INVALID_VALUE);
		m_pRcThrottle = pRC[iRcThrottle];

		return true;
	}

	int _APmavlink_drive::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ModuleBase::link(j, pM));
		string n;

		n = "";
		= j.value("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL__(m_pAP);

		return true;
	}

	int _APmavlink_drive::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_drive::check(void)
	{
		NULL__(m_pAP);
		NULL__(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmavlink_drive::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateDrive();

			ON_PAUSE;
		}
	}

	void _APmavlink_drive::onPause(void)
	{
		this->_ModuleBase::onPause();

		IF_(!m_bRcChanOverride);
		*m_pRcYaw = 0;
		*m_pRcThrottle = 0;
		m_pAP->getMavlink()->rcChannelsOverride(m_rcOverride);
	}

	bool _APmavlink_drive::updateDrive(void)
	{
		IF_F(!check());

		if (m_bSetYawSpeed)
		{
			m_pAP->getMavlink()->clNavSetYawSpeed(m_steer,
												  m_speed,
												  m_yawMode);
		}

		if (m_bRcChanOverride)
		{
			if (m_pRcYaw)
			{
				*m_pRcYaw = constrain(m_steer * m_pwmD + m_pwmM,
									  m_pwmM - m_pwmD,
									  m_pwmM + m_pwmD);
			}

			if (m_pRcThrottle)
			{
				*m_pRcThrottle = constrain(m_speed * m_pwmD + m_pwmM,
										   m_pwmM - m_pwmD,
										   m_pwmM + m_pwmD);
			}

			m_pAP->getMavlink()->rcChannelsOverride(m_rcOverride);
		}

		return true;
	}

	void _APmavlink_drive::setSteerSpeed(float steer, float spd)
	{
		m_steer = steer;
		m_speed = spd;
	}

	void _APmavlink_drive::setYawMode(bool bRelative)
	{
		if (bRelative)
			m_yawMode = 1.0;
		else
			m_yawMode = 0.0;
	}

	void _APmavlink_drive::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		((_Console *)pConsole)->addMsg("steer=" + f2str(m_steer) + ", speed=" + f2str(m_speed));

		NULL_(m_pRcYaw);
		NULL_(m_pRcThrottle);
		((_Console *)pConsole)->addMsg("yawMode=" + f2str(m_yawMode) + ", yaw=" + i2str(*m_pRcYaw) + ", throttle=" + i2str(*m_pRcThrottle));
	}

}
