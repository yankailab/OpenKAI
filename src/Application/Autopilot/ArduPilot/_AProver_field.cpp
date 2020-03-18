#include "_AProver_field.h"

namespace kai
{

_AProver_field::_AProver_field()
{
	m_pAP = NULL;
	m_pDrive = NULL;

	m_pPIDhdg = NULL;
	m_pDetBB = NULL;
	m_pDetSB = NULL;

	m_nSpeed = 0.0;
	m_bBlockBorder = false;
	m_dHdg = 0.0;
	m_sideBorder = 0.0;
	m_sideBorderTarget = 0.5;
	m_vSideBorderRange.x = 0.2;
	m_vSideBorderRange.y = 0.8;

	m_iPinLED = 21;
	m_iPinShutter = 10;
}

_AProver_field::~_AProver_field()
{
}

bool _AProver_field::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nSpeed", &m_nSpeed);
	pK->v("sideBorderTarget", &m_sideBorderTarget);
	pK->v("vSideBorderRange", &m_vSideBorderRange);
	pK->v<uint8_t>("iPinLED", &m_iPinLED);
	pK->v<uint8_t>("iPinShutter", &m_iPinShutter);

	string iName;
	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_AProver_drive", &iName);
	m_pDrive = (_AProver_drive*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pDrive, iName + ": not found");

	iName = "";
	pK->v("PIDhdg", &iName);
	m_pPIDhdg = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPIDhdg, iName + ": not found");

	iName = "";
	pK->v("_DetBB", &iName);
	m_pDetBB = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pDetBB, iName + ": not found");

	iName = "";
	pK->v("_DetSB", &iName);
	m_pDetSB = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pDetSB, iName + ": not found");

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
	NULL__(m_pDrive, -1);
	NULL__(m_pDetBB, -1);
	NULL__(m_pDetSB, -1);
	NULL__(m_pPIDhdg, -1);

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

void _AProver_field::updateDrive(void)
{
	IF_(check() < 0);

	uint16_t rcMode = m_pAP->m_pMavlink->m_mavMsg.m_rc_channels_raw.chan8_raw;
	if(m_pAP->getApMode() != AP_ROVER_GUIDED || rcMode == UINT16_MAX)
	{
		m_pDrive->setSpeed(0.0);
		m_pMC->transit("STANDBY");
	}

	string mission = m_pMC->getMissionName();

	//mode
	if(mission == "STANDBY")
	{
		if(rcMode < 1250)
			m_pMC->transit("FORWARD");
		else if(rcMode > 1750)
			m_pMC->transit("BACKWARD");
	}

	//speed
	float nSpeed;
	if(mission == "FORWARD")
		nSpeed = m_nSpeed;
	else if(mission == "BACKWARD")
		nSpeed = -m_nSpeed;
	else
		nSpeed = 0.0;

	m_pDrive->setSpeed(nSpeed);

	//field detection
	findBlockBoarder();
	findSideBoarder();
}

void _AProver_field::findBlockBoarder(void)
{
	if(m_pDetBB->size() > 0)
	{
		if (!m_bBlockBorder)
		{
			m_pMC->transit("BB");
		}
		m_bBlockBorder = true;
		return;
	}

	m_bBlockBorder = false;
}

void _AProver_field::findSideBoarder(void)
{
	IF_(check() < 0);

	OBJECT* pO;
	int i=0;
	while((pO = m_pDetSB->at(i++)) != NULL)
	{
		float border = pO->m_bb.midY();
		IF_CONT(border < m_vSideBorderRange.x);
		IF_CONT(border > m_vSideBorderRange.y);

		m_sideBorder = border;
		m_dHdg = m_pPIDhdg->update(m_sideBorder, m_sideBorderTarget, m_tStamp);

		m_pDrive->setYaw(m_dHdg);
		return;
	}
}

void _AProver_field::draw(void)
{
	this->_AutopilotBase::draw();

	addMsg("dHdg=" + f2str(m_dHdg) + ", sideBoarder=" + f2str(m_sideBorder));
}

}
