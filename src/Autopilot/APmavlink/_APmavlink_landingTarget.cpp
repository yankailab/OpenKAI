#include "_APmavlink_landingTarget.h"

namespace kai
{

	_APmavlink_landingTarget::_APmavlink_landingTarget()
	{
		m_pDS = nullptr;
		m_pU = nullptr;

		m_bHdg = false;
		m_bHdgMoving = false;
		m_hdgSp = 0.0;
		m_dHdg = 0.0;

		m_lt.angle_x = 0;
		m_lt.angle_y = 0;
		m_lt.distance = 0;
		m_lt.size_x = 0;
		m_lt.size_y = 0;
	}

	_APmavlink_landingTarget::~_APmavlink_landingTarget()
	{
	}

	bool _APmavlink_landingTarget::init(const json &j)
	{
		IF_F(!this->_APmavlink_move::init(j));

		m_vPsp = j.value("vPsp", vector<float>{0.5, 0.5});
		m_bHdg = j.value("bHdg", false);
		m_hdgDz = j.value("hdgDz", 10.0);
		m_hdgDzNav = m_hdgDz / 2;
		m_hdgDzNav = j.value("hdgDzNav", m_hdgDzNav);

		m_hTouchdown = j.value("hTouchdown", 0.0);
		m_kP = j.value("kP", 1.0);
		m_defaultDtgt = j.value("defaultDtgt", 2.0);

		m_vFov = j.value("vFov", vector<float>{90, 60});
		m_vFov *= DEG_2_RAD;

		m_yawRate = j.value("yawRate", 30);
		m_yawRate *= DEG_2_RAD;

		const json &jc = j.at("tags");
		IF__(!jc.is_object(), true);

		for (auto it = jc.begin(); it != jc.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			AP_LANDING_TARGET_TAG t;
			t.m_id = Ji.value("id", t.m_id);
			t.m_priority = Ji.value("priority", t.m_priority);
			m_vTags.push_back(t);
		}

		return true;
	}

	bool _APmavlink_landingTarget::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_APmavlink_move::link(j, pM));

		string n;

		n = j.value("_DistSensorBase", "");
		m_pDS = (_DistSensorBase *)pM->findModule(n);

		n = j.value("_Universe", "");
		m_pU = (_Universe *)pM->findModule(n);

		return true;
	}

	bool _APmavlink_landingTarget::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _APmavlink_landingTarget::check(void)
	{
		NULL_F(m_pDS);
		NULL_F(m_pU);

		return this->_APmavlink_move::check();
	}

	void _APmavlink_landingTarget::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateLandingTarget();
		}
	}

	void _APmavlink_landingTarget::updateLandingTarget(void)
	{
		IF_(!check());

		if (!findTag())
		{
			if (m_pAP->getMode() == AP_COPTER_GUIDED)
			{
				setHold();
				m_pAP->setMode(AP_COPTER_RTL);
			}

			return;
		}

		m_dHdg = dHdg(m_hdgSp, m_oTarget.getAttitude().x);
		float dHdgAbs = abs(m_dHdg);

		if (m_bHdgMoving)
		{
			if (dHdgAbs > m_hdgDzNav)
			{
				setHdg(0, (m_dHdg > 0) ? m_yawRate : (-m_yawRate));
				return;
			}

			setHold();
			m_bHdgMoving = false;
		}
		else if (m_bHdg)
		{
			if (dHdgAbs > m_hdgDz)
			{
				if (m_pAP->getMode() == AP_COPTER_LAND ||
					m_pAP->getMode() == AP_COPTER_RTL)
					m_pAP->setMode(AP_COPTER_GUIDED);

				m_bHdgMoving = true;
				return;
			}
		}

		if (m_pAP->getMode() == AP_COPTER_GUIDED)
			m_pAP->setMode(AP_COPTER_RTL);

		// vFloat3 vP = m_oTarget.getPos();
		// m_lt.angle_x = (vP.x - m_vPsp.x) * m_vFov.x * m_kP;
		// m_lt.angle_y = (vP.y - m_vPsp.y) * m_vFov.y * m_kP;
		// m_lt.size_x = m_oTarget.getWidth() * m_vFov.x;
		// m_lt.size_y = m_oTarget.getHeight() * m_vFov.y;
		// m_lt.position_valid = 0;

		// float h = m_pDS->d(0);
		// if(h > 0)
		// {
		// 	IF_(h < m_hTouchdown);
		// 	m_lt.distance = h;
		// }
		// else
		// {
		// 	m_lt.distance = m_defaultDtgt;
		// }

		// m_pAP->getMavlink()->landingTarget(m_lt);
	}

	bool _APmavlink_landingTarget::findTag(void)
	{
		IF_F(!check());

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

	AP_LANDING_TARGET_TAG *_APmavlink_landingTarget::getTag(int id)
	{
		for (int i = 0; i < m_vTags.size(); i++)
		{
			IF_CONT(m_vTags[i].m_id != id);
			return &m_vTags[i];
		}

		return NULL;
	}

	void _APmavlink_landingTarget::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_APmavlink_move::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		pC->addMsg("vAngle = (" + f2str(m_lt.angle_x) + ", " + f2str(m_lt.angle_y) + ")");
		pC->addMsg("dHdg = " + f2str(m_dHdg));
		pC->addMsg("Dist = " + f2str(m_lt.distance));
		pC->addMsg("vSize = (" + f2str(m_lt.size_x) + ", " + f2str(m_lt.size_y) + ")");
		pC->addMsg("iTag = " + i2str(m_oTarget.getTopClass()));
	}

}
