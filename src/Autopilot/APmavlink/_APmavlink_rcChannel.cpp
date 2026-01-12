#include "_APmavlink_rcChannel.h"

namespace kai
{

	_APmavlink_rcChannel::_APmavlink_rcChannel()
	{
		m_pAP = nullptr;

		m_rcMode.update();
		m_rcStickV.update();
		m_rcStickH.update();
	}

	_APmavlink_rcChannel::~_APmavlink_rcChannel()
	{
	}

	int _APmavlink_rcChannel::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

		= j.value("iRCmodeChan", &m_rcMode.m_iChan);
		pK->a("vRCmodeDiv", &m_rcMode.m_vDiv);
		= j.value("iRCstickV", &m_rcStickV.m_iChan);
		= j.value("iRCstickH", &m_rcStickH.m_iChan);

		pK->a("vRCmodeDiv", &m_rcStickV.m_vDiv);
		pK->a("vRCmodeDiv", &m_rcStickH.m_vDiv);

		m_rcMode.update();
		m_rcStickV.update();
		m_rcStickH.update();

		return true;
	}

	int _APmavlink_rcChannel::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ModuleBase::link(j, pM));

		string n = "";

		= j.value("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		NULL__(m_pAP);

		return true;
	}

	int _APmavlink_rcChannel::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_rcChannel::check(void)
	{
		NULL__(m_pAP);
		NULL__(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmavlink_rcChannel::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateRCchannel();

		}
	}

	void _APmavlink_rcChannel::updateRCchannel(void)
	{
		IF_(!check());

		uint16_t pwm;

		// pwm = m_pAP->getMavlink()->m_rcChannels.getRC ( m_rcMode.m_iChan );
		// IF_ ( pwm == UINT16_MAX );
		// m_rcMode.pwm ( pwm );
		// int iMode = m_rcMode.i();

		pwm = m_pAP->getMavlink()->m_rcChannels.getRC(m_rcStickV.m_iChan);
		IF_(pwm == UINT16_MAX);
		m_rcStickV.set(pwm);

		pwm = m_pAP->getMavlink()->m_rcChannels.getRC(m_rcStickH.m_iChan);
		IF_(pwm == UINT16_MAX);
		m_rcStickH.set(pwm);

	}

	void _APmavlink_rcChannel::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(!check());
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iMode: " + i2str(m_rcMode.i()), 1);
		pC->addMsg("stickV = " + f2str(m_rcStickV.v()), 1);
		pC->addMsg("stickH = " + f2str(m_rcStickH.v()), 1);
	}

}
