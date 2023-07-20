#include "_AP_goto.h"

namespace kai
{

	_AP_goto::_AP_goto()
	{
		m_pAP = NULL;
		m_pAPtarget = NULL;
		m_bTarget = false;
		m_iRelayLED = 0;

		m_apMount.init();
	}

	_AP_goto::~_AP_goto()
	{
	}

	bool _AP_goto::init(void *pKiss)
	{
		IF_F(!this->_AP_move::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iRelayLED", &m_iRelayLED);

		Kiss *pG = pK->child("mount");
		if (!pG->empty())
		{
			double p = 0, r = 0, y = 0;
			pG->v("pitch", &p);
			pG->v("roll", &r);
			pG->v("yaw", &y);

			m_apMount.m_control.input_a = p * 100; //pitch
			m_apMount.m_control.input_b = r * 100; //roll
			m_apMount.m_control.input_c = y * 100; //yaw
			m_apMount.m_control.save_position = 0;

			pG->v("stabPitch", &m_apMount.m_config.stab_pitch);
			pG->v("stabRoll", &m_apMount.m_config.stab_roll);
			pG->v("stabYaw", &m_apMount.m_config.stab_yaw);
			pG->v("mountMode", &m_apMount.m_config.mount_mode);
		}

		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		IF_Fl(!m_pAP, n + ": not found");

		n = "";
		pK->v("_AP_target_base", &n);
		m_pAPtarget = (_AP_base *)(pK->getInst(n));

		return true;
	}

	bool _AP_goto::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_goto::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);
		NULL__(m_pAPtarget, -1);
		NULL__(m_pAPtarget->m_pMav, -1);

		return this->_AP_move::check();
	}

	void _AP_goto::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			this->_AP_move::update();
			updateGoto();

			m_pT->autoFPSto();
		}
	}

	bool _AP_goto::updateGoto(void)
	{
		IF_F(check() < 0);

		m_bTarget = findTarget();
		m_pAP->m_pMav->clDoSetRelay(m_iRelayLED, m_bTarget);

		IF_F(!bActive());

		if (m_apMount.m_bEnable)
			m_pAP->setMount(m_apMount);

		if (!m_bTarget)
		{
			stop();
			return false;
		}

		setPosGlobal();
		return true;
	}

	bool _AP_goto::findTarget(void)
	{
		IF_F(check() < 0);

		IF_F(!m_pAPtarget->m_pMav->m_globalPositionINT.bReceiving(m_pT->getTfrom()));

		vDouble4 vAPpos = m_pAPtarget->getGlobalPos();
		IF_F(vAPpos.x <= 0.0);
		IF_F(vAPpos.y <= 0.0);

		m_vPspGlobal.x = vAPpos.x;
		m_vPspGlobal.y = vAPpos.y;

		return true;
	}

}
