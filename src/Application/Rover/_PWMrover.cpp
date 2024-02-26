#include "_PWMrover.h"

namespace kai
{

	_PWMrover::_PWMrover()
	{
		m_pDV = NULL;
		m_pU = NULL;
		m_pPWM = NULL;

		m_iClass = 0;
		m_vPWM.set(1000, 2000);
		m_pwmM = 1500;
		m_targetX = 0.5;
		m_Kstr = 0.0;
		m_targetD = 1.0;
		m_Kspd = 0.0;
		m_vKmotor.set(1.0);
		m_vSpd.set(0,500);
		m_minTarea = 0.25;

		m_d = 0;
		m_pwmL = m_pwmM;
		m_pwmR = m_pwmM;
	}

	_PWMrover::~_PWMrover()
	{
	}

	bool _PWMrover::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;
		pK->m_pInst = this;

		pK->v("iClass", &m_iClass);
		pK->v("vPWM", &m_vPWM);
		pK->v("pwmM", &m_pwmM);
		pK->v("targetX", &m_targetX);
		pK->v("Kstr", &m_Kstr);
		pK->v("targetD", &m_targetX);
		pK->v("Kspd", &m_Kspd);
		pK->v("vKmotor", &m_vKmotor);
		pK->v("vSpd", &m_vSpd);
		pK->v("minTarea", &m_minTarea);

		pK->v("pwmL", &m_pwmL);
		pK->v("pwmR", &m_pwmR);

		string n;

		n = "";
		pK->v("_RGBDbase", &n);
		m_pDV = (_RGBDbase *)(pK->getInst(n));
		IF_Fl(!m_pDV, n + ": not found");

		n = "";
		pK->v("_Universe", &n);
		m_pU = (_Universe *)(pK->getInst(n));
		IF_Fl(!m_pU, n + ": not found");

		n = "";
		pK->v("_PWMio", &n);
		m_pPWM = (_PWMio *)(pK->getInst(n));
		IF_Fl(!m_pPWM, n + ": not found");

		return true;
	}

	bool _PWMrover::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _PWMrover::check(void)
	{
		NULL__(m_pDV, -1);
		NULL__(m_pU, -1);
		NULL__(m_pPWM, -1);

		return this->_ModuleBase::check();
	}

	void _PWMrover::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			updateDrive();

			m_pT->autoFPSto();
		}
	}

	bool _PWMrover::updateDrive(void)
	{
		IF_F(check() < 0);

		//find target
		_Object *tO = NULL;
		_Object *pO;
		float minD = FLT_MAX;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			if (m_iClass >= 0)
			{
				IF_CONT(pO->getTopClass() != m_iClass);
			}

			vFloat4 bb = pO->getBB2D();
			IF_CONT(bb.area() < m_minTarea);
			float d = m_pDV->d(bb);
			IF_CONT(d <= 0);
			IF_CONT(d > minD);

			tO = pO;
			minD = d;
		}

		if (tO)
		{
			float x = tO->getX();
			float str = m_Kstr * (x - m_targetX);
			m_d = minD;
			float spd = 0.0;
			spd = m_Kspd * (m_d - m_targetD);
			spd = m_vSpd.constrain(spd);

			m_pwmL = m_vPWM.constrain(m_pwmM + spd + str * m_vKmotor.x);
			m_pwmR = m_vPWM.constrain(m_pwmM + spd + str * m_vKmotor.y);
		}
		else
		{
			m_d = 0.0;
			m_pwmL = m_pwmM;
			m_pwmR = m_pwmM;
		}

		m_pPWM->set(0, m_pwmL);
		m_pPWM->set(1, m_pwmR);

		return true;
	}

	void _PWMrover::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("d: " + f2str(m_d));
		pC->addMsg("pwmL: " + i2str(m_pwmL));
		pC->addMsg("pwmR: " + i2str(m_pwmR));
	}

	void _PWMrover::draw(void *pFrame)
	{
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame*)pFrame;

		Mat *pM = pF->m();
		IF_(pM->empty());

		float tX = pM->cols * m_targetX;
		line(*pM,
			 Point((int)tX, 0),
			 Point((int)tX, pM->rows),
			 Scalar(0, 0, 255), 2);
#endif
	}

}
