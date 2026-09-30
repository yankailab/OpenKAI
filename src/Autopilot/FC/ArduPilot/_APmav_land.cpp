#include "_APmav_land.h"

namespace kai
{

	_APmav_land::_APmav_land()
	{
		m_vDSrange.setZero();

		m_vFov = Vector2f(60, 60);

		m_vComplete = Vector4f(0.1, 0.1, 0.3, 3.0);
	}

	_APmav_land::~_APmav_land()
	{
	}

	bool _APmav_land::loadConfig(void)
	{
		IF_F(!this->_APmav_follow::loadConfig());
		const json &j = *m_pJ;

		jKv<float>(j, "vDSrange", m_vDSrange);
		jKv<float>(j, "vFov", m_vFov);
		jKv<float>(j, "vComplete", m_vComplete);
		jKv(j, "zrK", m_zrK);

		// uint64_t ieHdg = NSEC_SEC;
		// jKv(j,"ieHdgNs",ieHdg);//""
		// m_ieHdgCmd.init(ieHdg);

		m_pTag = nullptr;
		m_vTags.clear();
		const json *pJc = jK(j, "tags");
		IF__(!pJc || !pJc->is_object(), true);
		const json &jc = *pJc;

		for (auto it = jc.begin(); it != jc.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			AP_LAND_TAG t;
			t.m_configKey = it.key();
			jKv(Ji, "id", t.m_id);
			jKv(Ji, "priority", t.m_priority);
			jKv<float>(Ji, "vSize", t.m_vSize);
			jKv<float>(Ji, "vKdist", t.m_vKdist);
			m_vTags.push_back(t);
		}

		return true;
	}

	bool _APmav_land::saveConfig(bool bExport)
	{
		IF_F(!_APmav_follow::saveConfig(false));

		json &j = *m_pJ;
		j["vDSrange"] = {m_vDSrange[0], m_vDSrange[1]};
		j["vFov"] = {m_vFov[0], m_vFov[1]};
		j["vComplete"] = {m_vComplete[0], m_vComplete[1], m_vComplete[2], m_vComplete[3]};
		j["zrK"] = m_zrK;

		json &tags = j["tags"];
		if (!tags.is_object())
		{
			tags = json::object();
		}
		for (size_t i = 0; i < m_vTags.size(); ++i)
		{
			AP_LAND_TAG &entry = m_vTags[i];
			if (entry.m_configKey.empty())
			{
				entry.m_configKey = std::to_string(i);
				while (tags.contains(entry.m_configKey))
				{
					entry.m_configKey += "_";
				}
			}
			json &value = tags[entry.m_configKey];
			if (!value.is_object())
			{
				value = json::object();
			}
			value["id"] = entry.m_id;
			value["priority"] = entry.m_priority;
			value["vSize"] = {entry.m_vSize[0], entry.m_vSize[1]};
			value["vKdist"] = {entry.m_vKdist[0], entry.m_vKdist[1]};
		}

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _APmav_land::link(InstanceMgr *pM)
	{
		IF_F(!this->_APmav_follow::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_DistSensorBase", n);
		m_pDS = (_DistSensorBase *)pM->findModule(n);

		return true;
	}

	bool _APmav_land::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_land::check(void)
	{
		NULL_F(m_pDS);

		return this->_APmav_follow::check();
	}

	void _APmav_land::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateMove();

			if (bComplete())
			{
				setHold();
			}

			ON_PAUSE;
		}
	}

	void _APmav_land::onPause(void)
	{
		this->_APmav_follow::onPause();
		m_pTag = nullptr;
	}

	bool _APmav_land::bComplete(void)
	{
		IF_F(!check());
		IF_F(!m_bTarget);

		// NEDH
		IF_F(abs(m_vPvar.x() - m_vPsp.x()) > m_vComplete.x());
		IF_F(abs(m_vPvar.y() - m_vPsp.y()) > m_vComplete.y());
		IF_F(abs(m_vPvar.z()) > m_vComplete.z());
		IF_F(abs(dHdg(m_vPvar.w(), m_vPsp.w())) > m_vComplete.z());

		return true;
	}

	void _APmav_land::updateMove(void)
	{
		IF_(!check());

		if (m_apMount.m_bEnable)
			m_pAP->setMount(m_apMount);

		m_bTarget = findTag();

		if (!m_bTarget)
		{
			m_vSpd.x() = 0;
			m_vSpd.y() = 0;
			m_vSpd.z() = m_vPsp.z();
			m_vSpd.w() = 0;
			setVlocal(m_vSpd, false, false);
			return;
		}

		// move command
		updatePID();
		setVlocal(m_vSpd, false, false);

		// change yaw command
		// IF_(!m_ieHdgCmd.update(m_pT->getTfromNs()));
		// IF_(abs(m_vSpd.w()) < m_vComplete.w());

		setHold();
		//		setHdg(m_vSpd.w() * DEG_2_RAD, 0, true, false);
		m_pT->sleepT(NSEC_SEC);
	}

	bool _APmav_land::findTag(void)
	{
		IF_F(!check());
		m_pTag = nullptr;
		m_vPvar = m_vPsp;

		vector<BBOX_OBJ> vObjects;
		Vector3f vDim;
		IF_F(!readTargetObjects(m_pBBin, vObjects, vDim));

		uint64_t tNow = getTns();
		uint64_t tNewest = 0;
		const BBOX_OBJ *pTarget = nullptr;
		int priority = INT_MAX;
		for (const BBOX_OBJ &object : vObjects)
		{
			IF_CONT(object.m_type != obj_tag || !object.m_vPos.allFinite() || !bTargetFresh(object, tNow));
			AP_LAND_TAG *pTag = getTag(object.getTopClassID());
			IF_CONT(!pTag || object.m_tStamp < tNewest);
			IF_CONT(object.m_tStamp == tNewest && pTag->m_priority > priority);
			IF_CONT(!getTargetBB(object, vDim, m_vTargetBB));

			pTarget = &object;
			tNewest = object.m_tStamp;
			m_pTag = pTag;
			priority = pTag->m_priority;
		}
		NULL_F(pTarget);

		// Tags store pixel center/radius and heading in pos.z (degrees).
		float x = pTarget->m_vPos.x() / vDim.x();
		float y = pTarget->m_vPos.y() / vDim.y();
		float area = (m_vTargetBB.z() - m_vTargetBB.x()) * (m_vTargetBB.w() - m_vTargetBB.y());
		float dTs = m_pT->getDtNs() * SEC_NSEC;
		float fX = m_fX.update(x, dTs);
		float fY = m_fY.update(y, dTs);
		float fA = m_fZ.update(area, dTs);
		float fH = m_fH.update(pTarget->m_vPos.z(), dTs);
		m_tTargetUpdate = pTarget->m_tStamp;

		// Convert normalized image position to world-relative position.
		m_vPvar.z() = m_pTag->getDist(fA);
		m_vPvar.x() = m_vPvar.z() * tan((fY - 0.5) * m_vFov.y() * DEG_2_RAD);
		m_vPvar.y() = m_vPvar.z() * tan((fX - 0.5) * m_vFov.x() * DEG_2_RAD);
		m_vPvar.w() = fH;

		return true;
	}

	AP_LAND_TAG *_APmav_land::getTag(int id)
	{
		for (AP_LAND_TAG &tag : m_vTags)
		{
			IF_CONT(tag.m_id != id);
			return &tag;
		}

		return NULL;
	}

	void _APmav_land::updatePID(void)
	{
		uint64_t tNow = getTns();
		float dTs = (!m_tLastPIDupdate) ? 0 : (tNow - m_tLastPIDupdate) * SEC_NSEC;
		m_tLastPIDupdate = tNow;

		m_vSpd.x() = (m_pPitch) ? m_pPitch->update(m_vPvar.x(), m_vPsp.x(), dTs) : 0;
		m_vSpd.y() = (m_pRoll) ? m_pRoll->update(m_vPvar.y(), m_vPsp.y(), dTs) : 0;

		float dH = dHdg<float>(m_vPsp.w(), m_vPvar.w());
		m_vSpd.w() = (m_pYaw) ? m_pYaw->update(dH, 0.0, dTs) : dH;

		if (m_pAlt)
		{
			m_vSpd.z() = m_pAlt->update(m_vPvar.z(), m_vPsp.z(), dTs);
		}
		else
		{
			float dX = m_vPvar.x() - m_vPsp.x();
			float dY = m_vPvar.y() - m_vPsp.y();
			float r = sqrt(dX * dX + dY * dY);
			m_vSpd.z() = m_vPsp.z() * constrain(1.0 - r * m_zrK, 0.0, 1.0);
		}
	}

}
