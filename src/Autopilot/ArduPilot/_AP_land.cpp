#include "_AP_land.h"

namespace kai
{

	_AP_land::_AP_land()
	{
		m_zrK = 1.0;
		m_dTarget = -1.0;
		m_dTargetComplete = 1.0;
		m_altComplete = 1.0;
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
		pK->v("altComplete", &m_altComplete);

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

			if (!updateTarget())
			{
				m_dTarget = -1.0;
			}

			m_pT->autoFPSto();
		}
	}

	bool _AP_land::updateTarget(void)
	{
		IF_F(check() < 0);
		IF_F(!bActive());

		if (m_apMount.m_bEnable)
			m_pAP->setMount(m_apMount);

		float alt = m_pAP->getGlobalPos().w; //relative altitude
		m_bTarget = findTarget();

		m_vKpid.set(1.0);
		if(m_bTarget)
		{
			float kD = 1.0 - (m_vKpidIn.constrain(m_dTarget) - m_vKpidIn.x)/m_vKpidIn.len();
			kD = m_vKpidOut.constrain(kD);
			m_vKpid.x = kD;
			m_vKpid.y = kD;
		}

		if (alt > m_altComplete)
		{
			// descending
			if (!m_bTarget)
				m_vP = m_vTargetP;
		}
		else
		{
			if(m_bTarget && m_dTarget < m_dTargetComplete)
			{
				// not yet close enough to the target marker, keep descending
			}
			else
			{
				// going to touch down
				m_pSC->transit("TOUCHDOWN");
				m_vP = m_vTargetP;
			}
		}

		setPosLocal();
		return true;
	}

	bool _AP_land::findTarget(void)
	{
		IF_F(check() < 0);

		//target
		_Object *tO = NULL;
		_Object *pO;
		float minW = FLT_MAX;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			if (m_iClass >= 0)
			{
				IF_CONT(pO->getTopClass() != m_iClass);
			}

			float w = pO->getWidth();
			IF_CONT(w > minW);

			tO = pO;
			minW = w;
		}

		float *pX, *pY, *pR, *pH;
		float dTs = m_pT->getDt() * USEC_2_SEC;

		if (tO)
		{
			float x = tO->getX();
			float y = tO->getY();
			float r = tO->getRadius();
			float h = tO->getRoll();

			pX = m_fX.update(&x, dTs);
			pY = m_fY.update(&y, dTs);
			pR = m_fR.update(&r, dTs);
			pH = m_fH.update(&h, dTs);
		}
		else
		{
			pX = m_fX.update(NULL, dTs);
			pY = m_fY.update(NULL, dTs);
			pR = m_fR.update(NULL, dTs);
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

		//heading
		m_vP.w = *pH;

		//distance
		m_dTarget = *pR; //temporal

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
		pC->addMsg("Pos=(" + f2str(c.x) + ", " + f2str(c.y) + "), d=" + f2str(m_dTarget), 1);
		pC->addMsg("Size=(" + f2str(m_vTargetBB.width()) + ", " + f2str(m_vTargetBB.height()) + ")", 1);
	}

	void _AP_land::cvDraw(void *pWindow)
	{
#ifdef USE_OPENCV
		NULL_(pWindow);
		this->_AP_follow::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getNextFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		Rect r = bb2Rect(bbScale(m_vTargetBB, pM->cols, pM->rows));
		rectangle(*pM, r, Scalar(0, 0, 255), 2);
#endif
	}

}
