#include "_APmav_copter.h"

namespace kai
{

	_APmav_copter::_APmav_copter()
	{
	}

	_APmav_copter::~_APmav_copter()
	{
	}

	bool _APmav_copter::loadConfig(void)
	{
		IF_F(!this->_APmav_base::loadConfig());

		return true;
	}

	bool _APmav_copter::saveConfig(bool bExport)
	{
		if (!_APmav_base::saveConfig(false))
		{
			return false;
		}

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _APmav_copter::link(void)
	{
		IF_F(!this->_APmav_base::link());

		return true;
	}

	bool _APmav_copter::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_copter::check(void)
	{
		NULL_F(m_pMav);

		return this->_APmav_base::check();
	}

	void _APmav_copter::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateApMavRecv();

			updateApMavSend();
		}
	}

	void _APmav_copter::updateApMavRecv(void)
	{
		this->_APmav_base::updateApMavRecv();

		IF_(!check());

		switch (m_customModeFC)
		{
		case AP_COPTER_GUIDED:
			m_modeFC = apMode_ok;
			break;
		case AP_COPTER_GUIDED_NOGPS:
			m_modeFC = apMode_ok;
			break;

		case AP_COPTER_LOITER:
			m_modeFC = apMode_fc;
			break;
		case AP_COPTER_AUTO:
			m_modeFC = apMode_fc;
			break;
		case AP_COPTER_RTL:
			m_modeFC = apMode_fc;
			break;
		case AP_COPTER_LAND:
			m_modeFC = apMode_fc;
			break;

		case AP_COPTER_STABILIZE:
			m_modeFC = apMode_manual;
			break;
		case AP_COPTER_ACRO:
			m_modeFC = apMode_manual;
			break;
		case AP_COPTER_ALT_HOLD:
			m_modeFC = apMode_manual;
			break;

		default:
			m_modeFC = apMode_unknown;
			break;
		}
	}

	void _APmav_copter::updateApMavSend(void)
	{
		this->_APmav_base::updateApMavSend();

		IF_(!check());
	}

	string _APmav_copter::getModeName(void)
	{
		uint8_t iModeR = m_customModeFC;
		if (iModeR >= AP_COPTER_N_CUSTOM_MODE)
			return "?";
		if (iModeR < 0)
			return "?";

		return AP_COPTER_CUSTOM_MODE_NAME[iModeR];
	}

	void _APmav_copter::takeOff(float alt)
	{
		IF_(!check());

		m_pMav->clNavTakeoff(alt);
	}

	void _APmav_copter::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_APmav_base::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		pC->addMsg("-Custom Mode-", 1);
		pC->addMsg("\tcustom mode=\t" + i2str(getCustomMode()) + "\t" + getModeName(), 1);
	}

	void _APmav_copter::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));
	}

}
