#include "_APmav_rcChannel.h"

namespace kai
{

	_APmav_rcChannel::_APmav_rcChannel()
	{
		m_rcMode.update();
		m_rcStickV.update();
		m_rcStickH.update();
	}

	_APmav_rcChannel::~_APmav_rcChannel()
	{
	}

	bool _APmav_rcChannel::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "iRCmodeChan", m_rcMode.m_iChan);
		jKv<int>(j, "vRCmodeDiv", m_rcMode.m_vDiv);
		jKv(j, "iRCstickV", m_rcStickV.m_iChan);
		jKv(j, "iRCstickH", m_rcStickH.m_iChan);

		jKv<int>(j, "vRCmodeDiv", m_rcStickV.m_vDiv);
		jKv<int>(j, "vRCmodeDiv", m_rcStickH.m_vDiv);

		m_rcMode.update();
		m_rcStickV.update();
		m_rcStickH.update();

		return true;
	}

	bool _APmav_rcChannel::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_APmav_base", n);
		m_pAP = (_APmav_base *)(pM->findModule(n));
		NULL_F(m_pAP);

		return true;
	}

	bool _APmav_rcChannel::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_rcChannel::check(void)
	{
		NULL_F(m_pAP);
		NULL_F(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmav_rcChannel::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateRCchannel();
		}
	}

	void _APmav_rcChannel::updateRCchannel(void)
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

	void _APmav_rcChannel::console(void *pConsole)
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
