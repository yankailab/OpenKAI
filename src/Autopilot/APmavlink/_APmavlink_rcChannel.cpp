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

	int _APmavlink_rcChannel::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iRCmodeChan", &m_rcMode.m_iChan);
		pK->a("vRCmodeDiv", &m_rcMode.m_vDiv);
		pK->v("iRCstickV", &m_rcStickV.m_iChan);
		pK->v("iRCstickH", &m_rcStickH.m_iChan);

		pK->a("vRCmodeDiv", &m_rcStickV.m_vDiv);
		pK->a("vRCmodeDiv", &m_rcStickH.m_vDiv);

		m_rcMode.update();
		m_rcStickV.update();
		m_rcStickH.update();

		return OK_OK;
	}

	int _APmavlink_rcChannel::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n = "";

		pK->v("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pK->findModule(n));
		NULL__(m_pAP, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _APmavlink_rcChannel::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_rcChannel::check(void)
	{
		NULL__(m_pAP, OK_ERR_NULLPTR);
		NULL__(m_pAP->m_pMav, OK_ERR_NULLPTR);

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
		IF_(check() != OK_OK);

		uint16_t pwm;

		// pwm = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcMode.m_iChan );
		// IF_ ( pwm == UINT16_MAX );
		// m_rcMode.pwm ( pwm );
		// int iMode = m_rcMode.i();

		pwm = m_pAP->m_pMav->m_rcChannels.getRC(m_rcStickV.m_iChan);
		IF_(pwm == UINT16_MAX);
		m_rcStickV.set(pwm);

		pwm = m_pAP->m_pMav->m_rcChannels.getRC(m_rcStickH.m_iChan);
		IF_(pwm == UINT16_MAX);
		m_rcStickH.set(pwm);

	}

	void _APmavlink_rcChannel::console(void *pConsole)
	{
		NULL_(pConsole);
		IF_(check() != OK_OK);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("iMode: " + i2str(m_rcMode.i()), 1);
		pC->addMsg("stickV = " + f2str(m_rcStickV.v()), 1);
		pC->addMsg("stickH = " + f2str(m_rcStickH.v()), 1);
	}

}
