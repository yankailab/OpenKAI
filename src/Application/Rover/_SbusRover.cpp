#include "_SbusRover.h"

namespace kai
{

	_SbusRover::_SbusRover()
	{
		m_pSbus = NULL;
		m_pD = NULL;
		m_pA = NULL;

		m_iMode = 5;
		m_iSpd = 2;
		m_iSteer = 0;
		m_iElev = 8;

		m_nMode = 0.5;
		m_vModeR.set(0.4, 0.6);
		m_nSpd = 0.5;
		m_nSteer = 0.5;
		m_nElev = 0.5;

		m_pDV = NULL;
		m_pU = NULL;

		m_iClass = 0;
		m_minTarea = 0.1;
		m_tArea = 0;
		m_tX = 0.5;
		m_targetX = 0.5;
		m_Kstr = 0.1;
		m_spdFollow = 0.1;
		m_tAreaStop = 0.4;
		m_vTbb.set(0.1, 0.1, 0.9, 0.6);
		m_dTarget = 0.0;
		m_dTargetStop = 1.5;
	}

	_SbusRover::~_SbusRover()
	{
	}

	bool _SbusRover::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iMode", &m_iMode);
		pK->v("iSpd", &m_iSpd);
		pK->v("iSteer", &m_iSteer);
		pK->v("iElev", &m_iElev);
		pK->v("vModeR", &m_vModeR);
		pK->v("nSpd", &m_nSpd);
		pK->v("nSteer", &m_nSteer);
		pK->v("nElev", &m_nElev);

		pK->v("iClass", &m_iClass);
		pK->v("minTarea", &m_minTarea);
		pK->v("targetX", &m_targetX);
		pK->v("Kstr", &m_Kstr);
		pK->v("spdFollow", &m_spdFollow);
		pK->v("tAreaStop", &m_tAreaStop);
		pK->v("vTbb", &m_vTbb);
		pK->v("dTargetStop", &m_dTargetStop);

		return true;
	}

	bool _SbusRover::link(void)
	{
		IF_F(!this->BASE::link());
		IF_F(!m_pT->link());
		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_SBus", &n);
		m_pSbus = (_SBus *)(pK->getInst(n));
		IF_Fl(!m_pSbus, n + ": not found");

		n = "";
		pK->v("_Drive", &n);
		m_pD = (_Drive *)(pK->getInst(n));
		IF_Fl(!m_pD, n + ": not found");

		n = "";
		pK->v("_ActuatorBase", &n);
		m_pA = (_ActuatorBase *)(pK->getInst(n));

		n = "";
		pK->v("_RGBDbase", &n);
		m_pDV = (_RGBDbase *)(pK->getInst(n));
		IF_Fl(!m_pDV, n + ": not found");

		n = "";
		pK->v("_Universe", &n);
		m_pU = (_Universe *)(pK->getInst(n));
		IF_Fl(!m_pU, n + ": not found");

		return true;
	}

	bool _SbusRover::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _SbusRover::check(void)
	{
		NULL__(m_pSbus, -1);
		NULL__(m_pD, -1);
		NULL__(m_pDV, -1);
		NULL__(m_pU, -1);

		return this->_StateBase::check();
	}

	void _SbusRover::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateDrive();

			m_pT->autoFPSto();
		}
	}

	bool _SbusRover::updateDrive(void)
	{
		IF_F(check() < 0);
		IF_F(!bActive());

		// Sbus input
		m_nSpd = m_pSbus->v(m_iSpd);
		m_nSteer = m_pSbus->v(m_iSteer);
		m_nMode = m_pSbus->v(m_iMode);

		// target following mode
		if (m_nMode > m_vModeR.x && m_nMode <= m_vModeR.y)
		{
			m_nSpd = 0.5;
			m_nSteer = 0.5;

			_Object tO;
			if (findTarget(&tO))
			{
				m_tArea = tO.area();
				if (m_tArea < m_tAreaStop)
				{
					m_tX = tO.getX();
					m_nSteer += m_Kstr * (m_tX - m_targetX);
					m_nSpd += m_spdFollow;
				}
			}
		}

		// collision avoid
		m_dTarget = m_pDV->d(m_vTbb);
		if (m_dTarget < m_dTargetStop)
		{
			// allow steering and backward only
			m_nSpd = constrain<float>(m_nSpd, 0.0, 0.5);
		}

		// SBus failsafe
		if(m_pSbus->bFailSafe())
		{
			m_nSpd = 0.5;
			m_nSteer = 0.5;
		}

		// motor command
		m_pD->setSpeed((m_nSpd - 0.5) * 2);
		m_pD->setSteering((m_nSteer - 0.5) * 2);

		// camera stand actuator
		if (m_pA)
		{
			m_nElev = m_pSbus->v(m_iElev);
			if (m_nElev < 0.4)
				m_pA->setPtarget(0, -100);
			else if (m_nElev > 0.6)
				m_pA->setPtarget(0, 100);
			else
				m_pA->setStop();
		}

		return true;
	}

	bool _SbusRover::findTarget(_Object *pObj)
	{
		IF_F(check() < 0);
		NULL_F(pObj);

		_Object *tO = NULL;
		_Object *pO;
		float maxA = 0.0;
//		float minD = FLT_MAX;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			if (m_iClass >= 0)
			{
				IF_CONT(pO->getTopClass() != m_iClass);
			}

			vFloat4 bb = pO->getBB2D();
			float a = bb.area();
			IF_CONT(a < m_minTarea);
			IF_CONT(a < maxA);
			// float d = m_pDV->d(bb);
			// IF_CONT(d <= 0);
			// IF_CONT(d > minD);

			tO = pO;
			maxA = a;
			//minD = d;
		}

		NULL_F(tO);

		*pObj = *tO;
		return true;
	}

	void _SbusRover::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);
		msgActive(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("nSpd: " + f2str(m_nSpd));
		pC->addMsg("nSteer: " + f2str(m_nSteer));
		pC->addMsg("nElev: " + f2str(m_nElev));
		pC->addMsg("tArea: " + f2str(m_tArea));
		pC->addMsg("tX: " + f2str(m_tX));
		pC->addMsg("targetX: " + f2str(m_targetX));
		pC->addMsg("dTarget: " + f2str(m_dTarget));
		pC->addMsg("nMode: " + f2str(m_nMode));
		pC->addMsg("vModeR: " + f2str(m_vModeR.x) + " ," + f2str(m_vModeR.y));
	}

	void _SbusRover::draw(void *pFrame)
	{
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();
		IF_(pM->empty());

		float tX = pM->cols * m_targetX;
		line(*pM,
			 Point((int)tX, 0),
			 Point((int)tX, pM->rows),
			 Scalar(0, 0, 255), 2);
		tX = pM->cols * m_tX;
		line(*pM,
			 Point((int)tX, 0),
			 Point((int)tX, pM->rows),
			 Scalar(0, 255, 255), 2);

		Rect rTbb = bb2Rect(bbScale(m_vTbb, pM->cols, pM->rows));
		rectangle(*pM, rTbb, Scalar(0, 255, 255), 1);

#endif
	}

}
