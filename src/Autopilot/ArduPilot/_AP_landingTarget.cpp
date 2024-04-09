#include "_AP_landingTarget.h"

namespace kai
{

	_AP_landingTarget::_AP_landingTarget()
	{
		m_pAP = NULL;
		m_pU = NULL;
		m_vFov.set(60, 60);

	}

	_AP_landingTarget::~_AP_landingTarget()
	{
	}

	bool _AP_landingTarget::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
    	
		pK->v("vFov", &m_vFov);

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
		IF_F(!this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)pK->getInst(n);

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
		NULL__(m_pU, -1);

		return this->_ModuleBase::check();
	}

	void _AP_landingTarget::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			updateLandingTarget();

			m_pT->autoFPSto();
		}
	}

	void _AP_landingTarget::updateLandingTarget(void)
	{
		IF_(check() < 0);

		IF_(!findTag());

		// convert position from screen to world relative
		// m_vPvar.z = (pTag) ? pTag->getDist(*pA) : 1.0;
		// m_vPvar.x = m_vPvar.z * tan((*pY - 0.5) * m_vFov.y * DEG_2_RAD);
		// m_vPvar.y = m_vPvar.z * tan((*pX - 0.5) * m_vFov.x * DEG_2_RAD);
		// m_vPvar.w = *pH;

		vFloat3 vP = m_oTarget.getPos();
		
		mavlink_landing_target_t lt;
		lt.angle_x = vP.x;
		lt.angle_y = vP.y;
//		lt.distance = m_pDS;
		lt.size_x = m_oTarget.getWidth();
		lt.size_y = m_oTarget.getHeight();
		lt.position_valid = 0;
		m_pAP->m_pMav->landingTarget(lt);

		// change yaw command
		// IF_(!m_ieHdgCmd.update(m_pT->getTfrom()));
		// IF_(abs(m_vSpd.w) < m_vComplete.w);
		//		setHdg(m_vSpd.w * DEG_2_RAD, 0, true, false);
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

}
