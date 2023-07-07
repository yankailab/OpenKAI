#include "_AP_land.h"

namespace kai
{

	_AP_land::_AP_land()
	{
		m_pDS = NULL;
		m_zrK = 1.0;
		m_dTarget = -1.0;
		m_dTargetComplete = 1.0;
		m_rAltComplete = 1.0;
		m_rTargetComplete = 0.1;
		m_bRtargetComplete = false;
	}

	_AP_land::~_AP_land()
	{
	}

	bool _AP_land::init(void *pKiss)
	{
		IF_F(!this->_AP_follow::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("zrK", &m_zrK);
		pK->v("dTargetComplete", &m_dTargetComplete);
		pK->v("rAltComplete", &m_rAltComplete);
		pK->v("rTargetComplete", &m_rTargetComplete);

		return true;
	}

	bool _AP_land::link(void)
	{
		IF_F(!this->_AP_follow::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_DistSensorBase", &n);
		m_pDS = (_DistSensorBase *)(pK->getInst(n));

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
		IF_F(m_dTarget > m_dTargetComplete);

		return true;
	}

	bool _AP_land::updateTarget(void)
	{
		IF_F(check() < 0);
		IF_F(!bActive());

		if (m_apMount.m_bEnable)
			m_pAP->setMount(m_apMount);


		// update distances
		m_bTarget = findTarget();
		m_rAlt = m_dTarget;
		float rTgt = m_dTarget;

		// sensor blocked or not detecting ground
		if(m_dTarget < 0.0)
		{
			m_rAlt = m_pAP->getGlobalPos().w; //relative altitude from AP
			rTgt = m_vKpidIn.y;
		}

		// verify if the relative alt is lower than the threshold for touch down
		if(bComplete())
		{
			m_vP = m_vTargetP;
			releaseCtrl();
			return true;
		}

		// adjust PID according to altitude
		m_vKpid.set(1.0);
		if (m_bTarget)
		{
			float kD = (m_vKpidIn.constrain(rTgt) - m_vKpidIn.x) / m_vKpidIn.len();
			kD = m_vKpidOut.constrain(kD);
			m_vKpid.x = kD;
			m_vKpid.y = kD;
		}
		else
		{
			m_vP = m_vTargetP;
		}

		// handling the navigation
		setPosLocal();
		return true;
	}

	bool _AP_land::findTarget(void)
	{
		IF_F(check() < 0);

		//target
		_Object *tO = NULL;
		_Object *pO;
		int IDmax = -1;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			int id = pO->getTopClass();
			IF_CONT(id < IDmax);

			IDmax = id;
			tO = pO;
		}

		float *pX, *pY, *pH;
		float dTs = m_pT->getDt() * USEC_2_SEC;

		if (tO)
		{
			float x = tO->getX();
			float y = tO->getY();
			float h = tO->getRoll(); //use Roll for Aruco!

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

		NULL_F(pX);

		//position
		m_vP.x = *pX;
		m_vP.y = *pY;
		float dX = m_vP.x - m_vTargetP.x;
		float dY = m_vP.y - m_vTargetP.y;
		float r = sqrt(dX * dX + dY * dY);
		m_vP.z = m_vTargetP.z * constrain(1.0 - r * m_zrK, 0.0, 1.0);

		//r to target
		m_bRtargetComplete = (r < m_rTargetComplete)?true:false;		

		//heading
		m_vP.w = *pH;

		//distance
		if(m_pDS)
			m_dTarget = m_pDS->d(0);
		else
			m_dTarget = -1.0;

		return true;
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
		pC->addMsg("dTarget = " + f2str(m_dTarget), 1);
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
