#include "_AP_land.h"

namespace kai
{

	_AP_land::_AP_land()
	{
		m_pDS = NULL;
		m_vDSrange.clear();

		m_pTag = NULL;
		m_vFov.set(60, 60);

		m_vComplete.set(0.1, 0.1, 0.3, 3.0);
		m_zrK = 1.0;
	}

	_AP_land::~_AP_land()
	{
	}

	bool _AP_land::init(void *pKiss)
	{
		IF_F(!this->_AP_follow::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vDSrange", &m_vDSrange);
		pK->v("vFov", &m_vFov);
		pK->v("vComplete", &m_vComplete);
		pK->v("zrK", &m_zrK);

		// int ieHdg = USEC_1SEC;
		// pK->v("ieHdgUsec", &ieHdg);
		// m_ieHdgCmd.init(ieHdg);

		Kiss *pKt = pK->child("tags");
		NULL_T(pKt);
		Kiss *pT;
		int i = 0;
		while (!(pT = pKt->child(i++))->empty())
		{
			AP_LAND_TAG t;
			pT->v("id", &t.m_id);
			pT->v("priority", &t.m_priority);
			pT->v("vSize", &t.m_vSize);
			pT->v("vKdist", &t.m_vKdist);

			m_vTags.push_back(t);
		}

		return true;
	}

	bool _AP_land::link(void)
	{
		IF_F(!this->_AP_follow::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_DistSensorBase", &n);
		m_pDS = (_DistSensorBase *)pK->getInst(n);

		return true;
	}

	bool _AP_land::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_land::check(void)
	{
		NULL__(m_pDS, -1);

		return this->_AP_follow::check();
	}

	void _AP_land::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			updateMove();

			if (bComplete())
			{
				stop();
			}

			ON_SLEEP;
			m_pT->autoFPSto();
		}
	}

	void _AP_land::onGoSleep(void)
	{
		this->_ModuleBase::onGoSleep();

		clearPID();
	}

	bool _AP_land::bComplete(void)
	{
		IF_F(check() < 0);
		IF_F(!m_bTarget);

		// NEDH
		IF_F(abs(m_vPvar.x - m_vPsp.x) > m_vComplete.x);
		IF_F(abs(m_vPvar.y - m_vPsp.y) > m_vComplete.y);
		IF_F(abs(m_vPvar.z) > m_vComplete.z);
		IF_F(abs(dHdg(m_vPvar.w, m_vPsp.w)) > m_vComplete.z);

		return true;
	}

	void _AP_land::updateMove(void)
	{
		IF_(check() < 0);

		if (m_apMount.m_bEnable)
			m_pAP->setMount(m_apMount);

		m_bTarget = findTag();

		if (!m_bTarget)
		{
			m_vSpd.x = 0;
			m_vSpd.y = 0;
			m_vSpd.z = m_vPsp.z;
			m_vSpd.w = 0;
			setVlocal(m_vSpd, false, false);
			return;
		}

		// move command
		updatePID();
		setVlocal(m_vSpd, false, false);

		// change yaw command
		// IF_(!m_ieHdgCmd.update(m_pT->getTfrom()));
		// IF_(abs(m_vSpd.w) < m_vComplete.w);

		stop();
		//		setHdg(m_vSpd.w * DEG_2_RAD, 0, true, false);
		m_pT->sleepT(USEC_1SEC);
	}

	bool _AP_land::findTag(void)
	{
		IF_N(check() < 0);

		// find target
		AP_LAND_TAG *pTag = NULL;
		int priority = INT_MAX;
		_Object *tO = NULL;
		int i = 0;
		_Object *pO;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			int id = pO->getTopClass();
			pTag = getTag(id);
			IF_CONT(!pTag);
			IF_CONT(pTag->m_priority > priority);

			tO = pO;
			priority = pTag->m_priority;
		}

		// filter the position
		float *pX, *pY, *pA, *pH;
		float dTs = m_pT->getDt() * USEC_2_SEC;
		if (tO)
		{
			float x = tO->getX();
			float y = tO->getY();
			float a = tO->area();
			float h = tO->getRoll(); // use Roll for Aruco!

			pX = m_fX.update(&x, dTs);
			pY = m_fY.update(&y, dTs);
			pA = m_fZ.update(&a, dTs);
			pH = m_fH.update(&h, dTs);

			m_vTargetBB = tO->getBB2D();
		}
		else
		{
			pX = m_fX.update(NULL, dTs);
			pY = m_fY.update(NULL, dTs);
			pA = m_fZ.update(NULL, dTs);
			pH = m_fH.update(NULL, dTs);
		}

		if (!pX || !pY || !pA || !pH)
		{
			m_vPvar = m_vPsp;
			return false;
		}

		// convert position from screen to world relative
		m_vPvar.z = (pTag) ? pTag->getDist(*pA) : 1.0;
		m_vPvar.x = m_vPvar.z * tan((*pY - 0.5) * m_vFov.y * DEG_2_RAD);
		m_vPvar.y = m_vPvar.z * tan((*pX - 0.5) * m_vFov.x * DEG_2_RAD);
		m_vPvar.w = *pH;

		return true;
	}

	AP_LAND_TAG *_AP_land::getTag(int id)
	{
		for (int i = 0; i < m_vTags.size(); i++)
		{
			IF_CONT(m_vTags[i].m_id != id);
			return &m_vTags[i];
		}

		return NULL;
	}

	void _AP_land::updatePID(void)
	{
		uint64_t tNow = getApproxTbootUs();
		float dTs = (!m_tLastPIDupdate) ? 0 : (tNow - m_tLastPIDupdate) * USEC_2_SEC;
		m_tLastPIDupdate = tNow;

		m_vSpd.x = (m_pPitch) ? m_pPitch->update(m_vPvar.x, m_vPsp.x, dTs) : 0;
		m_vSpd.y = (m_pRoll) ? m_pRoll->update(m_vPvar.y, m_vPsp.y, dTs) : 0;

		float dH = dHdg<float>(m_vPsp.w, m_vPvar.w);
		m_vSpd.w = (m_pYaw) ? m_pYaw->update(dH, 0.0, dTs) : dH;

		if (m_pAlt)
		{
			m_vSpd.z = m_pAlt->update(m_vPvar.z, m_vPsp.z, dTs);
		}
		else
		{
			float dX = m_vPvar.x - m_vPsp.x;
			float dY = m_vPvar.y - m_vPsp.y;
			float r = sqrt(dX * dX + dY * dY);
			m_vSpd.z = m_vPsp.z * constrain(1.0 - r * m_zrK, 0.0, 1.0);
		}
	}

}
