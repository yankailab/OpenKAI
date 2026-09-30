#include "_APmav_follow.h"

namespace kai
{

	_APmav_follow::_APmav_follow()
	{
		m_tOutTargetNotFound.reStart(0);
		m_tOutTargetNotFound.setTout(NSEC_SEC / 10);
		m_vTargetBB.setZero();

		m_vPvar.setZero();
		m_vPsp.setZero();
		m_vSpd.setZero();

		m_apMount.init();
	}

	_APmav_follow::~_APmav_follow()
	{
	}

	bool _APmav_follow::loadConfig(void)
	{
		IF_F(!this->_APmav_move::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "iClass", m_iClass);
		jKv<float>(j, "vPsp", m_vPsp);

		uint64_t tOutTargetNotFound;
		if (jKv(j, "tOutTargetNotFound", tOutTargetNotFound))
			m_tOutTargetNotFound.setTout(tOutTargetNotFound);
		m_tOutTargetNotFound.reStart(0);

		int nWmed = 0;
		jKv(j, "nWmed", nWmed);
		float kTpred = 0.0f;
		jKv(j, "kTpred", kTpred);

		IF_F(!m_fX.init(nWmed, kTpred));
		IF_F(!m_fY.init(nWmed, kTpred));
		IF_F(!m_fZ.init(nWmed, kTpred));
		IF_F(!m_fH.init(nWmed, kTpred));

		const json *pJm = jK(j, "mount");
		IF__(!pJm || !pJm->is_object(), true);
		const json &jm = *pJm;

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

	bool _APmav_follow::saveConfig(bool bExport)
	{
		IF_F(!_APmav_move::saveConfig(false));

		json &j = *m_pJ;
		j["iClass"] = m_iClass;
		j["vPsp"] = {m_vPsp[0], m_vPsp[1], m_vPsp[2], m_vPsp[3]};
		j["tOutTargetNotFound"] = m_tOutTargetNotFound.m_tOut;
		j["nWmed"] = m_fX.m_nWmed;
		j["kTpred"] = m_fX.m_kTpred;

		json &mount = j["mount"];
		if (!mount.is_object())
		{
			mount = json::object();
		}
		mount["bEnable"] = m_apMount.m_bEnable;
		mount["pitch"] = m_apMount.m_control.input_a / 100.0;
		mount["roll"] = m_apMount.m_control.input_b / 100.0;
		mount["yaw"] = m_apMount.m_control.input_c / 100.0;
		mount["stabPitch"] = m_apMount.m_config.stab_pitch;
		mount["stabRoll"] = m_apMount.m_config.stab_roll;
		mount["stabYaw"] = m_apMount.m_config.stab_yaw;
		mount["mountMode"] = m_apMount.m_config.mount_mode;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _APmav_follow::link(InstanceMgr *pM)
	{
		IF_F(!this->_APmav_move::link(pM));
		const json &j = *m_pJ;

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
		m_pTracker = dynamic_cast<_TrackerBase *>(static_cast<_ModuleBase *>(pM->findModule(n)));
		IF_Le_F(!n.empty() && !m_pTracker, "_TrackerBase not found: " + n);

		n = "";
		jKv(j, "BBoxStreamIn", n);
		m_pBBin = dynamic_cast<BBoxStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(m_pBBin == nullptr, "BBoxStreamIn not found: " + n);

		n = "";
		jKv(j, "BBoxStreamTrackIn", n);
		m_pBBtrackin = dynamic_cast<BBoxStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F((m_pTracker || !n.empty()) && !m_pBBtrackin, "BBoxStreamTrackIn not found: " + n);
		IF_Le_F(m_pTracker && m_pBBtrackin == m_pBBin, "Detection and tracking BBoxStreams must be distinct");

		return true;
	}

	bool _APmav_follow::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_follow::check(void)
	{
		NULL_F(m_pBBin);
		IF_F(m_pTracker && (!m_pBBtrackin || m_pBBtrackin == m_pBBin));

		return this->_APmav_move::check();
	}

	void _APmav_follow::update(void)
	{
		while (m_pT->bRun())
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
				m_vSpd.setZero();
			}

			ON_PAUSE;
		}
	}

	void _APmav_follow::onPause(void)
	{
		clearPID();
		m_bTarget = false;
		m_tTargetUpdate = 0;
		m_tLastDetection = 0;
		m_tTrackStart = 0;
		m_tTrackWatermark = 0;
		m_tOutTargetNotFound.reStart(0);
		if (m_pTracker)
			m_pTracker->stopTrack();
	}

	bool _APmav_follow::updateTarget(void)
	{
		IF_F(!check());

		if (m_apMount.m_bEnable)
			m_pAP->setMount(m_apMount);

		bool bFound = findTarget();

		// Seed once per new detection observation; old tracking history stays intact.
		if (m_pTracker)
		{
			if (bFound && m_tTargetUpdate > m_tLastDetection)
			{
				m_tLastDetection = m_tTargetUpdate;
				if (m_pTracker->startTrack(m_vTargetBB))
				{
					m_tTrackStart = m_tTargetUpdate;
					m_tTrackWatermark = m_tTrackStart;
					vector<BBOX_OBJ> vPrevious;
					m_pBBtrackin->get(vPrevious);
					uint64_t tNow = getTns();
					for (const BBOX_OBJ &object : vPrevious)
					{
						IF_CONT(!bTargetFresh(object, tNow));
						m_tTrackWatermark = std::max(m_tTrackWatermark, object.m_tStamp);
					}
				}
			}

			vector<BBOX_OBJ> vTracked;
			Vector3f vDim;
			if (m_tTrackStart && readTargetObjects(m_pBBtrackin, vTracked, vDim))
			{
				uint64_t tNow = getTns();
				uint64_t tNewest = bFound ? m_tTargetUpdate : 0;
				int topProb = INT_MAX;
				for (const BBOX_OBJ &object : vTracked)
				{
					IF_CONT(!bTargetFresh(object, tNow));
					IF_CONT(object.m_tStamp <= m_tTrackStart || object.m_tStamp <= m_tTrackWatermark);
					IF_CONT(object.m_tStamp < tNewest);
					IF_CONT(object.m_tStamp == tNewest && object.getTopClassProb() < topProb);
					IF_CONT(!getTargetBB(object, vDim, m_vTargetBB));
					tNewest = object.m_tStamp;
					topProb = object.getTopClassProb();
					m_tTargetUpdate = object.m_tStamp;
					bFound = true;
				}
			}
		}

		uint64_t tNow = getTns();
		if (bFound)
		{
			// Re-reading history must not extend the selected observation's deadline.
			m_tOutTargetNotFound.reStart(m_tTargetUpdate);
			m_bTarget = true;
		}
		else
		{
			m_bTarget = m_tOutTargetNotFound.bStarted() && !m_tOutTargetNotFound.bTout(tNow);
		}

		if (!m_bTarget)
		{
			if (m_pTracker && m_tTrackStart)
				m_pTracker->stopTrack();
			m_tTrackStart = 0;
			m_tTrackWatermark = 0;
			m_fY.reset();
			m_fX.reset();
			m_fZ.reset();
			m_fH.reset();
			m_vPvar.setZero();
			m_vTargetBB.setZero();
			return false;
		}

		// NEDH (PRAH) order
		float dT = nsec2sec<float>(m_pT->getDtNs());
		m_vPvar.x() = m_fY.update(((m_vTargetBB.y() + m_vTargetBB.w()) / 2), dT);
		m_vPvar.y() = m_fX.update(((m_vTargetBB.x() + m_vTargetBB.z()) / 2), dT);
		m_vPvar.z() = m_fZ.update(m_vPsp.z(), dT);
		m_vPvar.w() = m_fH.update(m_vPsp.w(), dT);

		return true;
	}

	bool _APmav_follow::readTargetObjects(BBoxStream *pStreamin, vector<BBOX_OBJ> &vObjects, Vector3f &vDim)
	{
		NULL_F(pStreamin);
		pStreamin->get(vObjects);
		vDim = pStreamin->getContainerDim();
		IF_F(!vDim.allFinite() || vDim.x() <= 0 || vDim.y() <= 0);
		return true;
	}

	bool _APmav_follow::bTargetFresh(const BBOX_OBJ &object, uint64_t tNow) const
	{
		// Zero disables loss grace, while allowing the normal 100 ms capture latency.
		uint64_t tMaxAge = m_tOutTargetNotFound.m_tOut ? m_tOutTargetNotFound.m_tOut : NSEC_SEC / 10;
		IF_F(!object.m_tStamp || object.m_tStamp > tNow || tNow - object.m_tStamp > tMaxAge);
		return true;
	}

	bool _APmav_follow::getTargetBB(const BBOX_OBJ &object, const Vector3f &vDim, Vector4f &vBB)
	{
		IF_F(!vDim.allFinite() || vDim.x() <= 0 || vDim.y() <= 0);
		IF_F(object.m_type != obj_bbox && object.m_type != obj_tag);
		Vector4f bb = object.getBB2D();
		IF_F(!bb.allFinite() || bb.z() <= bb.x() || bb.w() <= bb.y());
		// PID setpoints and tracker commands use normalized image coordinates.
		vBB = bb.cwiseQuotient(Vector4f(vDim.x(), vDim.y(), vDim.x(), vDim.y()));
		return true;
	}

	bool _APmav_follow::findTarget(void)
	{
		IF_F(!check());

		vector<BBOX_OBJ> vObjects;
		Vector3f vDim;
		IF_F(!readTargetObjects(m_pBBin, vObjects, vDim));

		uint64_t tNow = getTns();
		uint64_t tNewest = 0;
		int topProb = -1;
		for (const BBOX_OBJ &object : vObjects)
		{
			IF_CONT(object.getTopClassID() != m_iClass || !bTargetFresh(object, tNow));
			IF_CONT(object.m_tStamp < tNewest);
			IF_CONT(object.m_tStamp == tNewest && object.getTopClassProb() < topProb);
			IF_CONT(!getTargetBB(object, vDim, m_vTargetBB));
			tNewest = object.m_tStamp;
			topProb = object.getTopClassProb();
		}

		IF_F(!tNewest);
		m_tTargetUpdate = tNewest;
		return true;
	}

	void _APmav_follow::updatePID(void)
	{
		uint64_t tNow = getTns();
		float dTs = (m_tLastPIDupdate == 0) ? 0 : ((float)(tNow - m_tLastPIDupdate)) * SEC_NSEC;
		m_tLastPIDupdate = tNow;

		m_vSpd.x() = (m_pPitch) ? m_pPitch->update(m_vPvar.x(), m_vPsp.x(), dTs) : 0;
		m_vSpd.y() = (m_pRoll) ? m_pRoll->update(m_vPvar.y(), m_vPsp.y(), dTs) : 0;
		m_vSpd.z() = (m_pAlt) ? m_pAlt->update(m_vPvar.z(), m_vPsp.z(), dTs) : 0;
		m_vSpd.w() = (m_pYaw) ? m_pYaw->update(dHdg<float>(m_vPsp.w(), m_vPvar.w()), 0.0, dTs) : 0;
	}

	void _APmav_follow::clearPID(void)
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

	void _APmav_follow::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_APmav_move::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		pC->addMsg("vPsp  = (" + f2str(m_vPsp.x()) + ", " + f2str(m_vPsp.y()) + ", " + f2str(m_vPsp.z()) + ", " + f2str(m_vPsp.w()) + ")", 1);
		pC->addMsg("vPvar = (" + f2str(m_vPvar.x()) + ", " + f2str(m_vPvar.y()) + ", " + f2str(m_vPvar.z()) + ", " + f2str(m_vPvar.w()) + ")", 1);
		pC->addMsg("vSpd  = (" + f2str(m_vSpd.x()) + ", " + f2str(m_vSpd.y()) + ", " + f2str(m_vSpd.z()) + ", " + f2str(m_vSpd.w()) + ")", 1);
		pC->addMsg("", 1);

		pC->addMsg("vTbb     = (" + f2str(m_vTargetBB.x()) + ", " + f2str(m_vTargetBB.y()) + ", " + f2str(m_vTargetBB.z()) + ", " + f2str(m_vTargetBB.w()) + ")", 1);
		Vector2f c = ((m_vTargetBB.head<2>() + m_vTargetBB.tail<2>()) / 2);
		pC->addMsg("vTcenter = (" + f2str(c.x()) + ", " + f2str(c.y()) + ")", 1);
		pC->addMsg("vTsize   = (" + f2str((m_vTargetBB.z() - m_vTargetBB.x())) + ", " + f2str((m_vTargetBB.w() - m_vTargetBB.y())) + ")", 1);
		pC->addMsg("vTarea   = " + f2str(std::abs((m_vTargetBB.z() - m_vTargetBB.x()) * (m_vTargetBB.w() - m_vTargetBB.y()))), 1);
		pC->addMsg("", 1);

		if (m_bTarget)
			pC->addMsg("Target found", 1);
		else
			pC->addMsg("Target not found", 1);
	}

	void _APmav_follow::draw(void *pMat)
	{
		NULL_(pMat);
		IF_(!check());

#ifdef USE_OPENCV
		Mat *pM = static_cast<Mat *>(pMat);
		IF_(pM->empty());
		IF_(!m_bTarget);

		Rect r = bb2Rect(bbScale(m_vTargetBB, pM->cols, pM->rows));
		rectangle(*pM, r, Scalar(255, 0, 0), 3);
#endif
	}

}
