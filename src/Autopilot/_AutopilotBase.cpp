#include "_AutopilotBase.h"

namespace kai
{

	_AutopilotBase::_AutopilotBase()
	{
	}

	_AutopilotBase::~_AutopilotBase()
	{
	}

	bool _AutopilotBase::init(const json &j)
	{
		IF_F(!this->_ReferenceFrame::init(j));

		jKv(j, "type", m_type);

		return true;
	}

	bool _AutopilotBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ReferenceFrame::link(j, pM));

		return true;
	}

	bool _AutopilotBase::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _AutopilotBase::check(void)
	{
		return this->_ReferenceFrame::check();
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
	}

	bool _AutopilotBase::setArm(AP_ARM a)
	{
		m_arm = a;
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

	const Vector4d &_AutopilotBase::getHomePos(void)
	{
		return m_vHomePos;
	}

	const Vector3f &_AutopilotBase::getVelocity(void)
	{
		return m_vVelocity;
	}

	float _AutopilotBase::getBattery(void)
	{
		return m_battery;
	}

	void _AutopilotBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ReferenceFrame::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("-State-", 1);
		pC->addMsg("\tARM:", getArm());

		pC->addMsg("-Mode-", 1);
		pC->addMsg("\tapMode:" + i2str(getMode()), 1);

		pC->addMsg("Global Pos-----------------------", 1);
		pC->addMsg("\tlat=\t" + lf2str(m_vPos.x(), 7) + "\tlon=\t" + lf2str(m_vPos.y(), 7), 1);
		pC->addMsg("\talt=\t" + lf2str(m_vPos.z(), 2) + "\trelAlt=\t" + lf2str(m_rAlt, 2), 1);

		pC->addMsg("-Home Pos-", 1);
		pC->addMsg("\tlat=\t" + f2str(m_vHomePos.x(), 7) + "\tlon=\t" + f2str(m_vHomePos.y(), 7) + "\talt=\t" + f2str(m_vHomePos.z(), 7), 1);

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
