#include "_AProver_field.h"

namespace kai
{

_AProver_field::_AProver_field()
{
	m_pAP = NULL;

	m_speed = 0.0;
	m_yaw = 0.0;
	m_yawMode = 1.0;

	m_pPIDhdg = NULL;
	m_pLineM = NULL;
	m_pLineL = NULL;

	m_dHdg = 0.0;
	m_nSpeed = 0.0;
	m_borderL = 0.0;
	m_vBorderLrange.x = 0.2;
	m_vBorderLrange.y = 0.8;
	m_borderLtarget = 0.5;
	m_kBorderLhdg = 3.0;
	m_vdHdgRange.x = -5.0;
	m_vdHdgRange.y = 5.0;

	m_bLineM = false;
	m_tCamShutter = 200000;
	m_tCamShutterStart = 0;

	m_iPinLEDtag = 21;
	m_iPinCamShutter = 10;
}

_AProver_field::~_AProver_field()
{
}

bool _AProver_field::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("speed", &m_speed);
	pK->v("yaw", &m_yaw);
	pK->v("yawMode", &m_yawMode);

	pK->v("vBorderLrange", &m_vBorderLrange);
	pK->v("borderLtarget", &m_borderLtarget);
	pK->v("kBorderLhdg", &m_kBorderLhdg);
	pK->v("vdHdgRange", &m_vdHdgRange);
	pK->v<float>("nSpeed", &m_nSpeed);
	pK->v<uint8_t>("iPinLEDtag", &m_iPinLEDtag);
	pK->v<uint8_t>("iPinCamShutter", &m_iPinCamShutter);
	pK->v("tCamShutter", &m_tCamShutter);

	string iName;
	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("PIDhdg", &iName);
	m_pPIDhdg = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPIDhdg, iName + ": not found");

	iName = "";
	F_ERROR_F(pK->v("_LineM", &iName));
	m_pLineM = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pLineM, iName + ": not found");

	iName = "";
	F_ERROR_F(pK->v("_LineL", &iName));
	m_pLineL = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pLineL, iName + ": not found");

	return true;
}

bool _AProver_field::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: " << retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AProver_field::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMavlink, -1);
	NULL__(m_pPIDhdg, -1);
	NULL__(m_pLineM, -1);
	NULL__(m_pLineL, -1);

	return this->_AutopilotBase::check();
}

void _AProver_field::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateDrive();

		this->autoFPSto();
	}
}

bool _AProver_field::updateDrive(void)
{
	IF_F(check() < 0);
	IF_F(!bActive());

	uint32_t apMode = m_pAP->getApMode();
	string mission = m_pMC->getMissionName();

//	if (mission == "IDLE")
//	{
//		if (m_pCMD->m_mode != rover_stop && m_pCMD->m_mode != rover_manual)
//			m_pMC->transit("MOVE");
//
//		m_pCMD->pinOut(m_iPinCamShutter, 0);
//		m_tCamShutterStart = 0;
//	}
//	else if (mission == "MOVE")
//	{
//		//		m_pR->setSpeed(nSpeed);
//
//		m_pCMD->pinOut(m_iPinCamShutter, 0);
//		m_tCamShutterStart = 0;
//	}
//	else if (mission == "TAG")
//	{
//		//		m_pR->setSpeed(0.0);
//
//		if (m_tCamShutterStart <= 0)
//		{
//			m_pCMD->pinOut(m_iPinCamShutter, 1);
//			m_tCamShutterStart = m_tStamp;
//		}
//		else if (m_tStamp - m_tCamShutterStart > m_tCamShutter)
//		{
//			m_pCMD->pinOut(m_iPinCamShutter, 0);
//			m_tCamShutterStart = m_tStamp;
//		}
//	}

	findLineM();
	findLineL();

	return true;
}

void _AProver_field::findLineM(void)
{
	IF_(check() < 0);

	OBJECT* pO;
	int i = 0;
	while ((pO = m_pLineM->at(i++)) != NULL)
	{
		if (!m_bLineM)
		{
			m_pMC->transit("TAG");
		}
		m_bLineM = true;
		return;
	}

	m_bLineM = false;
}

void _AProver_field::findLineL(void)
{
	IF_(check() < 0);

//	if (m_pLineL->size() > 0)
//		m_pCMD->pinOut(m_iPinLEDtag, 1);
//	else
//		m_pCMD->pinOut(m_iPinLEDtag, 0);

	OBJECT* pO;
	int i = 0;
	while ((pO = m_pLineL->at(i++)) != NULL)
	{
		float border = pO->m_bb.midY();
		IF_CONT(border < m_vBorderLrange.x);
		IF_CONT(border > m_vBorderLrange.y);

		m_borderL = border;
		m_dHdg = constrain((m_borderL - m_borderLtarget) * m_kBorderLhdg,
				m_vdHdgRange.x, m_vdHdgRange.y);

//		m_pR->setTargetHdgDelta(m_dHdg);
		return;
	}

}

void _AProver_field::draw(void)
{
	this->_AutopilotBase::draw();
	string msg = *this->getName() + ": dHdg=" + f2str(m_dHdg) + ", borderL="
			+ f2str(m_borderL);
	addMsg(msg);
}

}
