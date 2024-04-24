#include "_AP_landingTarget.h"

namespace kai
{

	_AP_landingTarget::_AP_landingTarget()
	{
		m_pDS = NULL;
		m_pU = NULL;
		m_vFov = 60 * DEG_2_RAD;
		m_vPsp.set(0.5, 0.5);

		m_yawRate = 30 * DEG_2_RAD;
		m_kP = 1.0;
		m_defaultDtgt = 2.0;

		m_bHdg = false;
		m_hdgSp = 0.0;
		m_hdgDz = 2.0;

		m_lt.angle_x = 0;
		m_lt.angle_y = 0;
		m_lt.distance = 0;
		m_lt.size_x = 0;
		m_lt.size_y = 0;
	}

	_AP_landingTarget::~_AP_landingTarget()
	{
	}

	bool _AP_landingTarget::init(void *pKiss)
	{
		IF_F(!this->_AP_move::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vPsp", &m_vPsp);
		pK->v("bHdg", &m_bHdg);
		pK->v("hdgDz", &m_hdgDz);
		pK->v("kP", &m_kP);
		pK->v("defaultDtgt", &m_defaultDtgt);

		if (pK->v("vFov", &m_vFov))
			m_vFov *= DEG_2_RAD;

		if (pK->v("yawRate", &m_yawRate))
			m_yawRate *= DEG_2_RAD;

		Kiss *pKt = pK->child("tags");
		NULL_T(pKt);
		Kiss *pT;
		int i = 0;
		while (!(pT = pKt->child(i++))->empty())
		{
			AP_LANDING_TARGET_TAG t;
			pT->v("id", &t.m_id);
			pT->v("priority", &t.m_priority);

			m_vTags.push_back(t);
		}

		return true;
	}

	bool _AP_landingTarget::link(void)
	{
		IF_F(!this->_AP_move::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_DistSensorBase", &n);
		m_pDS = (_DistSensorBase *)pK->getInst(n);

		n = "";
		pK->v("_Universe", &n);
		m_pU = (_Universe *)pK->getInst(n);

		return true;
	}

	bool _AP_landingTarget::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_landingTarget::check(void)
	{
		NULL__(m_pDS, -1);
		NULL__(m_pU, -1);

		return this->_AP_move::check();
	}

	void _AP_landingTarget::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateLandingTarget();

			m_pT->autoFPSto();
		}
	}

	void _AP_landingTarget::updateLandingTarget(void)
	{
		IF_(check() < 0);

		if(!findTag())
		{
			stop();
			return;
		}

		// adjust heading if needed
		if (m_bHdg)
		{
			float dH = dHdg(m_hdgSp, m_oTarget.getRoll()); // TODO
			if (abs(dH) > m_hdgDz)
			{
				if(m_pAP->getApMode() == AP_COPTER_LAND)
					m_pAP->setApMode(AP_COPTER_GUIDED);

				setHdg(0, (dH > 0) ? m_yawRate : (-m_yawRate));
				return;
			}

			stop();
		}

		if(m_pAP->getApMode() == AP_COPTER_GUIDED)
			m_pAP->setApMode(AP_COPTER_LAND);

		vFloat3 vP = m_oTarget.getPos();
		m_lt.angle_x = (vP.x - m_vPsp.x) * m_vFov.x * m_kP;
		m_lt.angle_y = (vP.y - m_vPsp.y) * m_vFov.y * m_kP;
		m_lt.size_x = m_oTarget.getWidth() * m_vFov.x;
		m_lt.size_y = m_oTarget.getHeight() * m_vFov.y;
		m_lt.position_valid = 0;

		float h = m_pDS->d(0);
		m_lt.distance = (h > 0) ? h : m_defaultDtgt;

		m_pAP->m_pMav->landingTarget(m_lt);
	}

	bool _AP_landingTarget::findTag(void)
	{
		IF_N(check() < 0);

		AP_LANDING_TARGET_TAG *pTag = NULL;
		int priority = INT_MAX;
		_Object *tO = NULL;
		_Object *pO;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			int id = pO->getTopClass();
			pTag = getTag(id);
			IF_CONT(!pTag);
			IF_CONT(pTag->m_priority > priority);

			tO = pO;
			priority = pTag->m_priority;
		}

		NULL_F(tO);
		m_oTarget = *tO;

		return true;
	}

	AP_LANDING_TARGET_TAG *_AP_landingTarget::getTag(int id)
	{
		for (int i = 0; i < m_vTags.size(); i++)
		{
			IF_CONT(m_vTags[i].m_id != id);
			return &m_vTags[i];
		}

		return NULL;
	}

	void _AP_landingTarget::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_AP_move::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		pC->addMsg("vAngle = (" + f2str(m_lt.angle_x) + ", " + f2str(m_lt.angle_y) + ")");
		pC->addMsg("Dist = " + f2str(m_lt.distance));
		pC->addMsg("vSize = (" + f2str(m_lt.size_x) + ", " + f2str(m_lt.size_y) + ")");
	}

}
