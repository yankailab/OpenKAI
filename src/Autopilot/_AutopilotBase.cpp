#include "_AutopilotBase.h"

namespace kai
{

	_AutopilotBase::_AutopilotBase()
	{
		m_wrMode.init(apMode_unknown, apMode_unknown);
		m_wrArm.init(apArm_unknown, apArm_unknown);

		m_ieSendHB.init(USEC_1SEC);
		m_ieSendMsgInt.init(USEC_1SEC);
	}

	_AutopilotBase::~_AutopilotBase()
	{
	}

	bool _AutopilotBase::init(const json &j)
	{
		IF_F(!this->_ReferenceFrame::init(j));

		jKv(j, "type", m_type);

		float t;
		if (jKv(j, "ieSendHB", t))
			m_ieSendHB.init(t * SEC_2_USEC);

		if (jKv(j, "ieSendMsgInt", t))
			m_ieSendMsgInt.init(t * SEC_2_USEC);

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

		if (!m_wrMode.bWritten())
		{
			setMode(m_wrMode.getWrite());
		}

		if (!m_wrArm.bWritten())
		{
			setArm(m_wrArm.getWrite());
		}
	}

	AP_TYPE _AutopilotBase::getType(void)
	{
		return m_type;
	}

	void _AutopilotBase::setMode(AP_MODE m)
	{
		m_wrMode.setWrite(m);
	}

	void _AutopilotBase::setArm(AP_ARM a)
	{
		m_wrArm.setWrite(a);
	}

	AP_MODE _AutopilotBase::getMode(void)
	{
		return m_wrMode.getRead();
	}

	AP_ARM _AutopilotBase::getArm(void)
	{
		return m_wrArm.getRead();
	}

	float _AutopilotBase::getRelativeAlt(void)
	{
		return m_rAlt;
	}

	const Vector4d& _AutopilotBase::getHomePos(void)
	{
		return m_vHomePos;
	}

	const Vector3f& _AutopilotBase::getSpeed(void)
	{
		return m_vSpeed;
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
		pC->addMsg("State-----------------------------", 1);
		pC->addMsg("\tARM:", getArm());

		pC->addMsg("Mode------------------------------", 1);
		pC->addMsg("\tapMode=" + i2str(getMode()), 1);

		pC->addMsg("Home Pos--------------------------", 1);
		pC->addMsg("\tlat=\t" + f2str(m_vHomePos.x(), 7) + "\tlon=\t" + f2str(m_vHomePos.y(), 7) + "\talt=\t" + f2str(m_vHomePos.z(), 7), 1);

		pC->addMsg("Speed-----------------------------", 1);
		pC->addMsg("\tvx=\t" + f2str(m_vSpeed.x()) +
					   "\tvy=\t" + f2str(m_vSpeed.y()) +
					   "\tvz=\t" + f2str(m_vSpeed.z()),
				   1);

		pC->addMsg("Battery-----------------------------", 1);
		pC->addMsg("\tbatt=\t" + f2str(m_battery));
	}

	void _AutopilotBase::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

	}

}
