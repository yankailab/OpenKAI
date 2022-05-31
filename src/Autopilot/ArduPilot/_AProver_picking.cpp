#include "_AProver_picking.h"

namespace kai
{

	_AProver_picking::_AProver_picking()
	{
		m_pAP = NULL;
		m_pDrive = NULL;
		m_pArm = NULL;
		m_rcMode.update();
		m_pArmMC = NULL;
	}

	_AProver_picking::~_AProver_picking()
	{
	}

	bool _AProver_picking::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("iRCmodeChan", &m_rcMode.m_iChan);
		pK->a("vRCmodeDiv", &m_rcMode.m_vDiv);
		m_rcMode.update();

		Kiss *pRC = pK->child("RC");
		int i = 0;
		while (1)
		{
			Kiss *pC = pRC->child(i++);
			if (pC->empty())
				break;

			RC_CHANNEL rc;
			pC->v("iChan", &rc.m_iChan);
			pC->v("pwmL", &rc.m_rawL);
			pC->v("pwmM", &rc.m_rawM);
			pC->v("pwmH", &rc.m_rawH);
			pC->a("vDiv", &rc.m_vDiv);
			rc.update();
			m_vRC.push_back(rc);
		}

		IF_F(!m_pSC);
		m_iState.STANDBY = m_pSC->getStateIdxByName("STANDBY");
		m_iState.MANUAL = m_pSC->getStateIdxByName("MANUAL");
		m_iState.AUTOPICK = m_pSC->getStateIdxByName("AUTOPICK");
		m_iState.AUTO = m_pSC->getStateIdxByName("AUTO");
		IF_F(!m_iState.bValid());

		string n;
		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)pK->getInst(n);
		IF_Fl(!m_pAP, n + ": not found");

		n = "";
		pK->v("_AProver_drive", &n);
		m_pDrive = (_AProver_drive *)pK->getInst(n);
		//	IF_Fl(!m_pDrive, n + ": not found");

		n = "";
		pK->v("_PickingArm", &n);
		m_pArm = (_PickingArm *)pK->getInst(n);
		NULL_Fl(m_pArm, n + ": not found");

		n = "";
		pK->v("_StateControlArm", &n);
		m_pArmMC = (_StateControl *)pK->getInst(n);
		NULL_Fl(m_pArmMC, n + ": not found");

		return true;
	}

	bool _AProver_picking::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AProver_picking::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);
		//	NULL__(m_pDrive, -1);
		NULL__(m_pArm, -1);
		NULL__(m_pArmMC, -1);

		return this->_StateBase::check();
	}

	void _AProver_picking::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			this->_StateBase::update();

			if (!updateDrive())
			{
				m_pSC->transit(m_iState.STANDBY);
				//			m_pDrive->setSpeed(0.0);
				//			m_pDrive->setYaw(0.0);
			}

			updatePicking();

			m_pT->autoFPSto();
		}
	}

	bool _AProver_picking::updateDrive(void)
	{
		IF_F(check() < 0);

		bool bArmed = m_pAP->bApArmed();
		uint32_t apMode = m_pAP->getApMode();
		uint16_t pwmMode = m_pAP->m_pMav->m_rcChannels.getRC(m_rcMode.m_iChan);
		string mission = m_pSC->getStateName();

		//	IF_F(!bArmed);
		//	IF_F(apMode == AP_ROVER_HOLD);
		IF_F(pwmMode == UINT16_MAX);

		m_rcMode.set(pwmMode);
		int iMode = m_rcMode.i();

		switch (iMode)
		{
		case 0:
			m_pSC->transit(m_iState.MANUAL);
			break;
		case 1:
			m_pSC->transit(m_iState.AUTOPICK);
			break;
		case 2:
			m_pSC->transit(m_iState.AUTO);
			break;
		}

		//m_pDrive->setSpeed(nSpeed);

		return true;
	}

	bool _AProver_picking::updatePicking(void)
	{
		IF_F(check() < 0);

		int i;

		for (i = 0; i < m_vRC.size(); i++)
		{
			RC_CHANNEL *pRC = &m_vRC[i];
			uint16_t r = m_pAP->m_pMav->m_rcChannels.getRC(pRC->m_iChan);
			if (r == UINT16_MAX)
				r = pRC->m_rawM;
			pRC->set(r);
		}

		int iM = m_pSC->getStateIdx();
		string armMission = m_pArmMC->getStateName();

		if (iM == m_iState.STANDBY)
		{
		}
		else if (iM == m_iState.MANUAL)
		{
			vFloat3 vM;
			vM.x = m_vRC[0].v();
			vM.y = m_vRC[1].v();
			vM.z = m_vRC[2].v();

			m_pArmMC->transit("EXTERNAL");
			m_pArm->move(vM);
			m_pArm->grip((m_vRC[4].i()) ? false : true);
			//		m_pArm->wakeUp();

			//		vM.init(-1.0);
			//		vM.x = m_vRC[3].v();
			//		m_pArm->rotate(vM);
		}
		else //AUTOPICK
		{
			if (armMission == "EXTERNAL")
				m_pArmMC->transit("RECOVER");
		}

		return true;
	}

	void _AProver_picking::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_StateBase::console(pConsole);
		IF_(check() < 0);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("RC mode: " + i2str(m_rcMode.i()));

		for (RC_CHANNEL rc : m_vRC)
			pC->addMsg("Chan" + i2str(rc.m_iChan) + ": " + i2str(rc.m_raw) + " | " + f2str(rc.v()) + " | " + i2str(rc.i()));
	}

}
