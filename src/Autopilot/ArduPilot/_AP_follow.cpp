#include "../ArduPilot/_AP_follow.h"

namespace kai
{

	_AP_follow::_AP_follow()
	{
		m_pU = NULL;
		m_pTracker = NULL;
		m_iClass = -1;
		m_bTarget = false;
		m_vTargetBB.clear();

		m_vPvar.clear();
		m_vPvar.x = 0.5;
		m_vPvar.y = 0.5;

		m_vPsp.clear();
		m_vPsp.x = 0.5;
		m_vPsp.y = 0.5;

		m_pPitch = NULL;
		m_pRoll = NULL;
		m_pAlt = NULL;
		m_pYaw = NULL;

		m_apMount.init();
	}

	_AP_follow::~_AP_follow()
	{
	}

	bool _AP_follow::init(void *pKiss)
	{
		IF_F(!this->_AP_move::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vPsp", &m_vPsp);

		int nWmed = 0;
		int nWpred = 0;
		float dThold = 0.0;
		pK->v("nWmed", &nWmed);
		pK->v("nWpred", &nWpred);
		pK->v("dThold", &dThold);

		IF_F(!m_fX.init(nWmed, nWpred, dThold));
		IF_F(!m_fY.init(nWmed, nWpred, dThold));
		IF_F(!m_fZ.init(nWmed, nWpred, dThold));
		IF_F(!m_fH.init(nWmed, nWpred, dThold));

		pK->v("iClass", &m_iClass);

		Kiss *pG = pK->child("mount");
		if (!pG->empty())
		{
			pG->v("bEnable", &m_apMount.m_bEnable);

			float p = 0, r = 0, y = 0;
			pG->v("pitch", &p);
			pG->v("roll", &r);
			pG->v("yaw", &y);

			m_apMount.m_control.input_a = p * 100; // pitch
			m_apMount.m_control.input_b = r * 100; // roll
			m_apMount.m_control.input_c = y * 100; // yaw
			m_apMount.m_control.save_position = 0;

			pG->v("stabPitch", &m_apMount.m_config.stab_pitch);
			pG->v("stabRoll", &m_apMount.m_config.stab_roll);
			pG->v("stabYaw", &m_apMount.m_config.stab_yaw);
			pG->v("mountMode", &m_apMount.m_config.mount_mode);
		}

		return true;
	}

	bool _AP_follow::link(void)
	{
		IF_F(!this->_AP_move::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("PIDpitch", &n);
		m_pPitch = (PID *)(pK->getInst(n));

		n = "";
		pK->v("PIDroll", &n);
		m_pRoll = (PID *)(pK->getInst(n));

		n = "";
		pK->v("PIDalt", &n);
		m_pAlt = (PID *)(pK->getInst(n));

		n = "";
		pK->v("PIDyaw", &n);
		m_pYaw = (PID *)(pK->getInst(n));

		n = "";
		pK->v("_TrackerBase", &n);
		m_pTracker = (_TrackerBase *)pK->getInst(n);

		n = "";
		pK->v("_Universe", &n);
		m_pU = (_Universe *)pK->getInst(n);

		return true;
	}

	bool _AP_follow::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_follow::check(void)
	{
		NULL__(m_pU, -1);

		return this->_AP_move::check();
	}

	void _AP_follow::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			// mavlink_command_int_t D;
			// D.command = 44001;
			// D.param1 = 3;
			// D.param2 = 1;
			// m_pAP->m_pMav->cmdInt(D);

			if (updateTarget())
			{
				updatePID();
				// setVlocal(m_vSpd);

				// temporal
				vDouble4 vPg = m_pAP->getGlobalPos();
				if (vPg.x + vPg.y > 0)
				{
					float apHdg = m_pAP->getApHdg();
					float s = sin(apHdg * DEG_2_RAD);
					float c = cos(apHdg * DEG_2_RAD);

					vDouble4 vP;
					vP.x = 35.7936444;	// vPg.x + m_vSpd.x * c - m_vSpd.y * s;
					vP.y = 140.2113273; // vPg.y + m_vSpd.x * s + m_vSpd.y * c;
					vP.z = vPg.z;
					vP.w = apHdg;
					//						doReposition(vP, sqrt(m_vSpd.x * m_vSpd.x + m_vSpd.y * m_vSpd.y), 0.1);
					doReposition(vP, 5.0, 0.1);
				}
			}
			else
			{
				stop();
				clearPID();
			}


			ON_SLEEP;
			m_pT->autoFPSto();
		}
	}

	void _AP_follow::onGoSleep(void)
	{
		this->_ModuleBase::onGoSleep();

		clearPID();
		m_bTarget = false;
		if (m_pTracker)
			m_pTracker->stopTrack();
	}

	bool _AP_follow::updateTarget(void)
	{
		IF_F(check() < 0);

		if (m_apMount.m_bEnable)
			m_pAP->setMount(m_apMount);

		m_bTarget = findTarget();

		// use tracker if available
		if (m_pTracker)
		{
			if (m_bTarget)
				m_pTracker->startTrack(m_vTargetBB);

			if (m_pTracker->trackState() == track_update)
			{
				m_vTargetBB = *m_pTracker->getBB();
				m_bTarget = true;
			}
		}

		// both detection and tracking failed
		IF_F(!m_bTarget);

		// NEDH (PRAH) order
		m_vPvar.x = m_vTargetBB.midY();
		m_vPvar.y = m_vTargetBB.midX();
		m_vPvar.z = m_vPsp.z;
		m_vPvar.w = m_vPsp.w;

		return true;
	}

	bool _AP_follow::findTarget(void)
	{
		IF_F(check() < 0);

		_Object *pO;
		_Object *tO = NULL;
		float topProb = 0.0;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			IF_CONT(pO->getTopClass() != m_iClass);
			IF_CONT(pO->getTopClassProb() < topProb);

			tO = pO;
			topProb = pO->getTopClassProb();
		}

		NULL_F(tO);
		m_vTargetBB = tO->getBB2D();

		return true;
	}

	void _AP_follow::updatePID(void)
	{
		uint64_t tNow = getApproxTbootUs();
		float dTs = (!m_tLastPIDupdate) ? 0 : ((float)(tNow - m_tLastPIDupdate)) * USEC_2_SEC;
		m_tLastPIDupdate = tNow;

		m_vSpd.x = (m_pPitch) ? m_pPitch->update(m_vPvar.x, m_vPsp.x, dTs) : 0;
		m_vSpd.y = (m_pRoll) ? m_pRoll->update(m_vPvar.y, m_vPsp.y, dTs) : 0;
		m_vSpd.z = (m_pAlt) ? m_pAlt->update(m_vPvar.z, m_vPsp.z, dTs) : 0;
		m_vSpd.w = (m_pYaw) ? m_pYaw->update(dHdg<float>(m_vPsp.w, m_vPvar.w), 0.0, dTs) : 0;
	}

	void _AP_follow::clearPID(void)
	{
		m_tLastPIDupdate = 0;

		if (m_pPitch)
			m_pPitch->reset();
		if (m_pRoll)
			m_pRoll->reset();
		if (m_pAlt)
			m_pAlt->reset();
		if (m_pYaw)
			m_pYaw->reset();
	}

	void _AP_follow::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_AP_move::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		if (!m_bTarget)
		{
			pC->addMsg("Target not found", 1);
			return;
		}

		pC->addMsg("vPsp  = (" + f2str(m_vPsp.x) + ", " + f2str(m_vPsp.y) + ", " + f2str(m_vPsp.z) + ", " + f2str(m_vPsp.w) + ")", 1);
		pC->addMsg("vPvar = (" + f2str(m_vPvar.x) + ", " + f2str(m_vPvar.y) + ", " + f2str(m_vPvar.z) + ", " + f2str(m_vPvar.w) + ")", 1);
		pC->addMsg("vSpd  = (" + f2str(m_vSpd.x) + ", " + f2str(m_vSpd.y) + ", " + f2str(m_vSpd.z) + ", " + f2str(m_vSpd.w) + ")", 1);

		pC->addMsg("", 1);

		pC->addMsg("vTbb  = (" + f2str(m_vTargetBB.x) + ", " + f2str(m_vTargetBB.y) + ", " + f2str(m_vTargetBB.z) + ", " + f2str(m_vTargetBB.w) + ")", 1);
		vFloat2 c = m_vTargetBB.center();
		pC->addMsg("vTc   = (" + f2str(c.x) + ", " + f2str(c.y) + ")", 1);
		pC->addMsg("vTs   = (" + f2str(m_vTargetBB.width()) + ", " + f2str(m_vTargetBB.height()) + ")", 1);
		pC->addMsg("vTa   = " + f2str(m_vTargetBB.area()), 1);
	}

	void _AP_follow::draw(void *pFrame)
	{
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_AP_move::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();
		IF_(pM->empty());

		Rect r = bb2Rect(bbScale(m_vTargetBB, pM->cols, pM->rows));
		rectangle(*pM, r, Scalar(0, 0, 255), 2);
#endif
	}

}
