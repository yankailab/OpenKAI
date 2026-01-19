#include "_APmavlink_follow.h"

namespace kai
{

	_APmavlink_follow::_APmavlink_follow()
	{
		m_pU = nullptr;
		m_pTracker = nullptr;
		m_tOutTargetNotFound.reStartT(0);
		m_tOutTargetNotFound.setTout(USEC_1SEC / 10);
		m_iClass = -1;
		m_bTarget = false;
		m_vTargetBB.clear();

		m_vPvar.clear();
		m_vPsp.clear();
		m_vSpd.set(0);
		m_tLastPIDupdate = 0;

		m_pPitch = nullptr;
		m_pRoll = nullptr;
		m_pAlt = nullptr;
		m_pYaw = nullptr;

		m_apMount.init();
	}

	_APmavlink_follow::~_APmavlink_follow()
	{
	}

	bool _APmavlink_follow::init(const json &j)
	{
		IF_F(!this->_APmavlink_move::init(j));

		jKv(j, "iClass", m_iClass);
		jKv<float>(j, "vPsp", m_vPsp);

		int tOutTargetNotFound;
		if (jKv(j, "tOutTargetNotFound", tOutTargetNotFound))
			m_tOutTargetNotFound.setTout(tOutTargetNotFound);
		m_tOutTargetNotFound.reStartT(0);

		int nWmed = 0;
		jKv(j, "nWmed", nWmed);
		int kTpred = 0;
		jKv(j, "kTpred", kTpred);

		IF_F(!m_fX.init(nWmed, kTpred));
		IF_F(!m_fY.init(nWmed, kTpred));
		IF_F(!m_fZ.init(nWmed, kTpred));
		IF_F(!m_fH.init(nWmed, kTpred));

		const json &jm = j.at("mount");
		IF__(!jm.is_object(), true);

		jKv(jm, "bEnable", m_apMount.m_bEnable);

		float p = 0, r = 0, y = 0;
		jKv(jm, "pitch", p);
		jKv(jm, "roll", r);
		jKv(jm, "yaw", y);
		m_apMount.m_control.input_a = p * 100; // pitch
		m_apMount.m_control.input_b = r * 100; // roll
		m_apMount.m_control.input_c = y * 100; // yaw
		m_apMount.m_control.save_position = 0;

		jKv(jm, "stabPitch", m_apMount.m_config.stab_pitch);
		jKv(jm, "stabRoll", m_apMount.m_config.stab_roll);
		jKv(jm, "stabYaw", m_apMount.m_config.stab_yaw);
		jKv(jm, "mountMode", m_apMount.m_config.mount_mode);

		return true;
	}

	bool _APmavlink_follow::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_APmavlink_move::link(j, pM));

		string n;

		n = "";
		jKv(j, "PIDpitch", n);
		m_pPitch = (PID *)(pM->findModule(n));

		n = "";
		jKv(j, "PIDroll", n);
		m_pRoll = (PID *)(pM->findModule(n));

		n = "";
		jKv(j, "PIDalt", n);
		m_pAlt = (PID *)(pM->findModule(n));

		n = "";
		jKv(j, "PIDyaw", n);
		m_pYaw = (PID *)(pM->findModule(n));

		n = "";
		jKv(j, "_TrackerBase", n);
		m_pTracker = (_TrackerBase *)pM->findModule(n);

		n = "";
		jKv(j, "_Universe", n);
		m_pU = (_Universe *)pM->findModule(n);

		return true;
	}

	bool _APmavlink_follow::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _APmavlink_follow::check(void)
	{
		NULL_F(m_pU);

		return this->_APmavlink_move::check();
	}

	void _APmavlink_follow::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			if (updateTarget())
			{
				updatePID();
				setVlocal(m_vSpd);
			}
			else
			{
				setHold();
				clearPID();
				m_vSpd.set(0);
			}

			ON_PAUSE;
		}
	}

	void _APmavlink_follow::onPause(void)
	{
		clearPID();
		m_bTarget = false;
		if (m_pTracker)
			m_pTracker->stopTrack();
	}

	bool _APmavlink_follow::updateTarget(void)
	{
		IF_F(!check());

		if (m_apMount.m_bEnable)
			m_pAP->setMount(m_apMount);

		bool bFound = findTarget();

		// use tracker if available
		if (m_pTracker)
		{
			if (bFound)
				m_pTracker->startTrack(m_vTargetBB);

			if (m_pTracker->trackState() == track_update)
			{
				m_vTargetBB = *m_pTracker->getBB();
				bFound = true;
			}
		}

		if (bFound)
		{
			// target found or tracked
			m_bTarget = true;
			m_tOutTargetNotFound.reStart();
		}
		else if (!m_tOutTargetNotFound.bTout())
		{
			// both detection and tracking failed but hold for timeout
			m_bTarget = true;
		}
		else
		{
			// both detection and tracking failed and time is out
			m_bTarget = false;
		}

		if (!m_bTarget)
		{
			m_fY.reset();
			m_fX.reset();
			m_fZ.reset();
			m_fH.reset();
			m_vPvar.set(0);
			m_vTargetBB.clear();
			return false;
		}

		// NEDH (PRAH) order
		float dT = usec2sec<float>(m_pT->getDt());
		m_vPvar.x = m_fY.update(m_vTargetBB.midY(), dT);
		m_vPvar.y = m_fX.update(m_vTargetBB.midX(), dT);
		m_vPvar.z = m_fZ.update(m_vPsp.z, dT);
		m_vPvar.w = m_fH.update(m_vPsp.w, dT);

		return true;
	}

	bool _APmavlink_follow::findTarget(void)
	{
		IF_F(!check());

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

	void _APmavlink_follow::updatePID(void)
	{
		uint64_t tNow = getTbootUs();
		float dTs = (m_tLastPIDupdate == 0) ? 0 : ((float)(tNow - m_tLastPIDupdate)) * USEC_2_SEC;
		m_tLastPIDupdate = tNow;

		m_vSpd.x = (m_pPitch) ? m_pPitch->update(m_vPvar.x, m_vPsp.x, dTs) : 0;
		m_vSpd.y = (m_pRoll) ? m_pRoll->update(m_vPvar.y, m_vPsp.y, dTs) : 0;
		m_vSpd.z = (m_pAlt) ? m_pAlt->update(m_vPvar.z, m_vPsp.z, dTs) : 0;
		m_vSpd.w = (m_pYaw) ? m_pYaw->update(dHdg<float>(m_vPsp.w, m_vPvar.w), 0.0, dTs) : 0;
	}

	void _APmavlink_follow::clearPID(void)
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

	void _APmavlink_follow::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_APmavlink_move::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		pC->addMsg("vPsp  = (" + f2str(m_vPsp.x) + ", " + f2str(m_vPsp.y) + ", " + f2str(m_vPsp.z) + ", " + f2str(m_vPsp.w) + ")", 1);
		pC->addMsg("vPvar = (" + f2str(m_vPvar.x) + ", " + f2str(m_vPvar.y) + ", " + f2str(m_vPvar.z) + ", " + f2str(m_vPvar.w) + ")", 1);
		pC->addMsg("vSpd  = (" + f2str(m_vSpd.x) + ", " + f2str(m_vSpd.y) + ", " + f2str(m_vSpd.z) + ", " + f2str(m_vSpd.w) + ")", 1);
		pC->addMsg("", 1);

		pC->addMsg("vTbb     = (" + f2str(m_vTargetBB.x) + ", " + f2str(m_vTargetBB.y) + ", " + f2str(m_vTargetBB.z) + ", " + f2str(m_vTargetBB.w) + ")", 1);
		vFloat2 c = m_vTargetBB.center();
		pC->addMsg("vTcenter = (" + f2str(c.x) + ", " + f2str(c.y) + ")", 1);
		pC->addMsg("vTsize   = (" + f2str(m_vTargetBB.width()) + ", " + f2str(m_vTargetBB.height()) + ")", 1);
		pC->addMsg("vTarea   = " + f2str(m_vTargetBB.area()), 1);
		pC->addMsg("", 1);

		if (m_bTarget)
			pC->addMsg("Target found", 1);
		else
			pC->addMsg("Target not found", 1);
	}

	void _APmavlink_follow::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_APmavlink_move::draw(pFrame);
		IF_(!check());

#ifdef USE_OPENCV
		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();
		IF_(pM->empty());
		IF_(!m_bTarget);

		Rect r = bb2Rect(bbScale(m_vTargetBB, pM->cols, pM->rows));
		rectangle(*pM, r, Scalar(255, 0, 0), 3);
#endif
	}

}
