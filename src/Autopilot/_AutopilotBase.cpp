#include "_AutopilotBase.h"

namespace kai
{

	_AutopilotBase::_AutopilotBase()
	{
	}

	_AutopilotBase::~_AutopilotBase()
	{
	}

	bool _AutopilotBase::loadConfig(void)
	{
		IF_F(!this->_ModuleBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "type", m_type);

		return true;
	}

	bool _AutopilotBase::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));

		json &j = *m_pJ;
		j["type"] = m_type;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _AutopilotBase::link(InstanceMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(pM));

		string tfName;
		jKv(*m_pJ, "_TransformRigid", tfName);
		m_pTf = dynamic_cast<_TransformRigid *>(static_cast<_ModuleBase *>(pM->findModule(tfName)));

		return true;
	}

	bool _AutopilotBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _AutopilotBase::check(void)
	{
		return this->_ModuleBase::check();
	}

	void _AutopilotBase::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateAutopilot();
		}
	}

	void _AutopilotBase::updateAutopilot(void)
	{
		IF_(!check());
	}

	AP_TYPE _AutopilotBase::getType(void)
	{
		return m_type;
	}

	bool _AutopilotBase::setMode(AP_MODE m)
	{
		m_mode = m;
		return true;
	}

	bool _AutopilotBase::setArm(AP_ARM a)
	{
		m_arm = a;
		return true;
	}

	AP_MODE _AutopilotBase::getMode(void)
	{
		return apMode_unknown;
	}

	AP_ARM _AutopilotBase::getArm(void)
	{
		return apArm_unknown;
	}

	float _AutopilotBase::getRelativeAlt(void)
	{
		return m_rAlt;
	}

	const Vector4d &_AutopilotBase::getPosGlobal(void)
	{
		return m_vPosGlobal;
	}

	const Vector3f &_AutopilotBase::getVelocity(void)
	{
		return m_vVelocity;
	}

	const Vector4d &_AutopilotBase::getPosHome(void)
	{
		return m_vPosHome;
	}

	float _AutopilotBase::getBattery(void)
	{
		return m_battery;
	}

	void _AutopilotBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("-State-", 1);
		const char *armMsg = "\tARM: unknown";
		switch (getArm())
		{
		case apArm_disarm:
			armMsg = "\tARM: disarmed";
			break;
		case apArm_arm:
			armMsg = "\tARM: armed";
			break;
		default:
			break;
		}
		pC->addMsg(armMsg, 1);

		pC->addMsg("-Mode-", 1);
		pC->addMsg("\tapMode:" + i2str(getMode()), 1);

		pC->addMsg("Global Pos-----------------------", 1);
		pC->addMsg("\tlat=\t" + lf2str(m_vPosGlobal.x(), 7) + "\tlon=\t" + lf2str(m_vPosGlobal.y(), 7), 1);
		pC->addMsg("\talt=\t" + lf2str(m_vPosGlobal.z(), 2) + "\trelAlt=\t" + lf2str(m_rAlt, 2), 1);
		pC->addMsg("\thdg=\t" + lf2str(m_vPosGlobal.w(), 2), 1);

		pC->addMsg("-Home Pos-", 1);
		pC->addMsg("\tlat=\t" + f2str(m_vPosHome.x(), 7) + "\tlon=\t" + f2str(m_vPosHome.y(), 7) + "\talt=\t" + f2str(m_vPosHome.z(), 7), 1);

		pC->addMsg("-Velocity-", 1);
		pC->addMsg("\tvx=\t" + f2str(m_vVelocity.x()) +
					   "\tvy=\t" + f2str(m_vVelocity.y()) +
					   "\tvz=\t" + f2str(m_vVelocity.z()),
				   1);

		pC->addMsg("-Battery-", 1);
		pC->addMsg("\tbatt=\t" + f2str(m_battery));
	}

	void _AutopilotBase::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));
	}

}
