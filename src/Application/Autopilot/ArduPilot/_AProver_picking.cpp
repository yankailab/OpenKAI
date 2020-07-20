#include "_AProver_picking.h"

#ifdef USE_OPENCV

namespace kai
{

_AProver_picking::_AProver_picking()
{
	m_pAP = NULL;
	m_pDrive = NULL;
	m_pPIDhdg = NULL;
	m_pArm = NULL;

	m_nSpeed = 0.0;
	m_iRCmode = 0;
}

_AProver_picking::~_AProver_picking()
{
}

bool _AProver_picking::init(void* pKiss)
{
	IF_F(!this->_MissionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nSpeed", &m_nSpeed);

	string iName;
	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*)pK->getInst(iName);
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_AProver_drive", &iName);
	m_pDrive = (_AProver_drive*)pK->getInst(iName);
	IF_Fl(!m_pDrive, iName + ": not found");

	iName = "";
	pK->v("PIDctrl", &iName);
	m_pPIDhdg = (PIDctrl*)pK->getInst(iName);
	NULL_Fl(m_pPIDhdg, iName + ": not found");

	iName = "";
	pK->v("_PickingArm", &iName);
	m_pArm = (_PickingArm*)pK->getInst(iName);
	NULL_Fl(m_pArm, iName + ": not found");

	return true;
}

bool _AProver_picking::start(void)
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

int _AProver_picking::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMav, -1);
	NULL__(m_pDrive, -1);
	NULL__(m_pPIDhdg, -1);
	NULL__(m_pArm, -1);

	return this->_MissionBase::check();
}

void _AProver_picking::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_MissionBase::update();

		if(!updateDrive())
		{
			m_pDrive->setSpeed(0.0);
			m_pDrive->setYaw(0.0);
			m_pMC->transit("STANDBY");
		}

		this->autoFPSto();
	}
}

bool _AProver_picking::updateDrive(void)
{
	IF_F(check() < 0);
	IF_F(!bActive());

	uint16_t rcMode = m_pAP->m_pMav->m_rcChannels.getRC(m_iRCmode);
	IF_F(m_pAP->getApMode() != AP_ROVER_GUIDED || rcMode == UINT16_MAX);

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

	return true;
}

void _AProver_picking::draw(void)
{
	this->_MissionBase::draw();
	IF_(check()<0);

//	addMsg("dHdg=" + f2str(m_dHdg) + ", sideBoarder=" + f2str(m_sideBorder));
}

}
#endif
