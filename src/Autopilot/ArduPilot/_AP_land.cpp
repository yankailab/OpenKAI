#include "_AP_land.h"

namespace kai
{

	_AP_land::_AP_land()
	{
		m_zrK = 1.0;
		m_rAltComplete = 1.0;
		m_rTargetComplete = 0.1;
		m_bRtargetComplete = false;

		m_pTag = NULL;
	}

	_AP_land::~_AP_land()
	{
	}

	bool _AP_land::init(void *pKiss)
	{
		IF_F(!this->_AP_follow::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("zrK", &m_zrK);
		pK->v("rAltComplete", &m_rAltComplete);
		pK->v("rTargetComplete", &m_rTargetComplete);

		int ieHdg = USEC_1SEC;
		pK->v("ieHdgUsec", &ieHdg);
		m_ieHdgCmd.init(ieHdg);

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

			m_vTags.push_back(t);
		}

		return true;
	}

	bool _AP_land::link(void)
	{
		IF_F(!this->_AP_follow::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		return true;
	}

	bool _AP_land::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_land::check(void)
	{
		return this->_AP_follow::check();
	}

	void _AP_land::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();
			this->_StateBase::update();

			updateTarget();

			m_pT->autoFPSto();
		}
	}

	bool _AP_land::bComplete(void)
	{
		IF_F(check() < 0);

		IF_F(m_rAlt > m_rAltComplete);
		IF_F(!m_bRtargetComplete);

		return true;
	}

	bool _AP_land::updateTarget(void)
	{
		IF_F(check() < 0);
		IF_F(!bActive());

		if (m_apMount.m_bEnable)
			m_pAP->setMount(m_apMount);

		m_pTag = findTag();
		m_rAlt = m_pAP->getGlobalPos().w; // relative altitude from AP

		if(m_ieHdgCmd.update(m_pT->getTfrom()))
		{
			//TODO: set global Yaw heading;
		}

		// verify the completion of descent procedure
		if (bComplete())
		{
			m_vPvar = m_vPsp;
			stop();
			return true;
		}

		// adjust PID according to altitude
		m_vKpid.set(1.0);
		if (m_pTag)
		{
			float kD = (m_vKpidIn.constrain(1.0 - m_pTag->m_K) - m_vKpidIn.x) / m_vKpidIn.len();
			kD = m_vKpidOut.constrain(kD);
			m_vKpid.x = kD;
			m_vKpid.y = kD;
		}

		// handling the move command
		setPosLocal();
		return true;
	}

	AP_LAND_TAG *_AP_land::findTag(void)
	{
		IF_N(check() < 0);

		// target
		_Object *tO = NULL;
		_Object *pO;
		AP_LAND_TAG* pTag;
		int i = 0;
		int priority = INT_MAX;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			int id = pO->getTopClass();
			pTag = getTag(id);
			IF_CONT(!pTag);
			IF_CONT(pTag->m_priority > priority);

			tO = pO;
			priority = pTag->m_priority;
		}

		float *pX, *pY, *pH;
		float dTs = m_pT->getDt() * USEC_2_SEC;
		if (tO)
		{
			float x = tO->getX();
			float y = tO->getY();
			float h = tO->getRoll(); // use Roll for Aruco!

			pX = m_fX.update(&x, dTs);
			pY = m_fY.update(&y, dTs);
			pH = m_fH.update(&h, dTs);
		}
		else
		{
			pX = m_fX.update(NULL, dTs);
			pY = m_fY.update(NULL, dTs);
			pH = m_fH.update(NULL, dTs);
		}

		NULL_N(pX);

		// position
		m_vPvar.x = *pX;
		m_vPvar.y = *pY;
		float dX = m_vPvar.x - m_vPsp.x;
		float dY = m_vPvar.y - m_vPsp.y;
		float r = sqrt(dX * dX + dY * dY);
		m_vPvar.z = m_vPsp.z * constrain(1.0 - r * m_zrK, 0.0, 1.0);

		// r to target
		m_bRtargetComplete = (r < m_rTargetComplete) ? true : false;

		// heading
		m_vPvar.w = *pH;

		return pTag;
	}

	AP_LAND_TAG *_AP_land::getTag(int id)
	{
		for(int i=0; i<m_vTags.size(); i++)
		{
			IF_CONT(m_vTags[i].m_id != id);
			return &m_vTags[i];
		}

		return NULL;
	}

	void _AP_land::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_AP_follow::console(pConsole);
		IF_(check() < 0);

		_Console *pC = (_Console *)pConsole;
		if (!m_bTarget)
		{
			pC->addMsg("Target not found", 1);
			return;
		}

		pC->addMsg("Target");
		vFloat2 c = m_vTargetBB.center();
		pC->addMsg("Pos = (" + f2str(c.x) + ", " + f2str(c.y), 1);
		pC->addMsg("Size = (" + f2str(m_vTargetBB.width()) + ", " + f2str(m_vTargetBB.height()) + ")", 1);
		pC->addMsg("rAlt = " + f2str(m_rAlt), 1);
	}

	void _AP_land::draw(void *pFrame)
	{
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_AP_follow::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame *)pFrame;

		Mat *pM = pF->m();
		IF_(pM->empty());

		Rect r = bb2Rect(bbScale(m_vTargetBB, pM->cols, pM->rows));
		rectangle(*pM, r, Scalar(0, 0, 255), 2);
#endif
	}

}
