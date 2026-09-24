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

	bool _APmav_land::init(const json &j)
	{
		IF_F(!this->_APmav_follow::init(j));

		jKv<float>(j, "vDSrange", m_vDSrange);
		jKv<float>(j, "vFov", m_vFov);
		jKv<float>(j, "vComplete", m_vComplete);
		jKv(j, "zrK", m_zrK);

		// int ieHdg = USEC_1SEC;
		// jKv(j,"ieHdgUsec",ieHdg);//""
		// m_ieHdgCmd.init(ieHdg);

		const json &jc = jK(j, "tags");
		IF__(!jc.is_object(), true);

		for (auto it = jc.begin(); it != jc.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			AP_LAND_TAG t;
			jKv(Ji, "id", t.m_id);
			jKv(Ji, "priority", t.m_priority);
			jKv<float>(Ji, "vSize", t.m_vSize);
			jKv<float>(Ji, "vKdist", t.m_vKdist);
			m_vTags.push_back(t);
		}

		return true;
	}

	bool _APmav_land::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_APmav_follow::link(j, pM));

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
		this->_ModuleBase::onPause();

		clearPID();
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
		// IF_(!m_ieHdgCmd.update(m_pT->getTfrom()));
		// IF_(abs(m_vSpd.w()) < m_vComplete.w());

		setHold();
		//		setHdg(m_vSpd.w() * DEG_2_RAD, 0, true, false);
		m_pT->sleepT(USEC_1SEC);
	}

	bool _APmav_land::findTag(void)
	{
		IF_F(!check());

		// find target
		AP_LAND_TAG *pTag = NULL;
		int priority = INT_MAX;
		_Object *tO = NULL;
		int i = 0;
		_Object *pO;
		while ((pO = m_pCanvas->get(i++)) != NULL)
		{
			int id = pO->getTopClass();
			pTag = getTag(id);
			IF_CONT(!pTag);
			IF_CONT(pTag->m_priority > priority);

			tO = pO;
			priority = pTag->m_priority;
		}

		// filter the position
		float fX, fY, fA, fH;
		float dTs = m_pT->getDt() * USEC_2_SEC;
		if (tO)
		{
			Vector3f vP = tO->getPos();
			float a = tO->getDimArea();

			Vector3f vA = tO->getAttitude();
			float h = vA.x(); // use Roll for Aruco!

			fX = m_fX.update(vP.x(), dTs);
			fY = m_fY.update(vP.y(), dTs);
			fA = m_fZ.update(a, dTs);
			fH = m_fH.update(h, dTs);

			m_vTargetBB = tO->getBB2D();
		}
		else
		{
			m_vPvar = m_vPsp;
			return false;
		}

		// convert position from screen to world relative
		m_vPvar.z() = (pTag) ? pTag->getDist(fA) : 1.0;
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
		uint64_t tNow = getApproxTbootUs();
		float dTs = (!m_tLastPIDupdate) ? 0 : (tNow - m_tLastPIDupdate) * USEC_2_SEC;
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
