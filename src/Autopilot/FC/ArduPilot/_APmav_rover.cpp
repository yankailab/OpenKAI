#include "_APmav_rover.h"

namespace kai
{

	_APmav_rover::_APmav_rover()
	{
	}

	_APmav_rover::~_APmav_rover()
	{
	}

	bool _APmav_rover::loadConfig(void)
	{
		IF_F(!this->_APmav_base::loadConfig());

		return true;
	}

	bool _APmav_rover::saveConfig(bool bExport)
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

	bool _APmav_rover::link(void)
	{
		IF_F(!this->_APmav_base::link());

		return true;
	}

	bool _APmav_rover::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_rover::check(void)
	{
		NULL_F(m_pMav);

		return this->_APmav_base::check();
	}

	void _APmav_rover::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateApMavRecv();

			updateApMavSend();
		}
	}

	void _APmav_rover::updateApMavRecv(void)
	{
		this->_APmav_base::updateApMavRecv();

		IF_(!check());

		switch (m_customModeFC)
		{
		case AP_ROVER_GUIDED:
			m_modeFC = apMode_ok;
			break;

		case AP_ROVER_HOLD:
			m_modeFC = apMode_fc;
			break;
		case AP_ROVER_LOITER:
			m_modeFC = apMode_fc;
			break;
		case AP_ROVER_FOLLOW:
			m_modeFC = apMode_fc;
			break;
		case AP_ROVER_AUTO:
			m_modeFC = apMode_fc;
			break;
		case AP_ROVER_RTL:
			m_modeFC = apMode_fc;
			break;
		case AP_ROVER_SMART_RTL:
			m_modeFC = apMode_fc;
			break;
		case AP_ROVER_INITIALISING:
			m_modeFC = apMode_fc;
			break;

		case AP_ROVER_MANUAL:
			m_modeFC = apMode_manual;
			break;
		case AP_ROVER_ACRO:
			m_modeFC = apMode_manual;
			break;
		case AP_ROVER_STEERING:
			m_modeFC = apMode_manual;
			break;

		default:
			m_modeFC = apMode_unknown;
			break;
		}
	}

	void _APmav_rover::updateApMavSend(void)
	{
		this->_APmav_base::updateApMavSend();

		IF_(!check());
	}

	string _APmav_rover::getModeName(void)
	{
		uint8_t iModeR = m_customModeFC;
		if (iModeR >= AP_ROVER_N_CUSTOM_MODE)
			return "?";
		if (iModeR < 0)
			return "?";

		return AP_ROVER_CUSTOM_MODE_NAME[iModeR];
	}

	void _APmav_rover::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_APmav_base::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		pC->addMsg("-Custom Mode-", 1);
		pC->addMsg("\tcustom mode=\t" + i2str(getCustomMode()) + "\t" + getModeName(), 1);
	}

	void _APmav_rover::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));
	}

}
