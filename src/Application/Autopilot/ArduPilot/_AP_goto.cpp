#include "_AP_goto.h"

namespace kai
{

_AP_goto::_AP_goto()
{
	m_pAP = NULL;
	m_pAPtarget = NULL;
	m_bTarget = false;
	m_iLEDpin = 12;

	m_apMount.init();
}

_AP_goto::~_AP_goto()
{
}

bool _AP_goto::init(void* pKiss)
{
	IF_F(!this->_AP_posCtrl::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("iLEDpin",&m_iLEDpin);

	Kiss* pG = pK->o("mount");
	if(!pG->empty())
	{
		double p=0, r=0, y=0;
		pG->v("pitch", &p);
		pG->v("roll", &r);
		pG->v("yaw", &y);

		m_apMount.m_control.input_a = p * 100;	//pitch
		m_apMount.m_control.input_b = r * 100;	//roll
		m_apMount.m_control.input_c = y * 100;	//yaw
		m_apMount.m_control.save_position = 0;

		pG->v("stabPitch", &m_apMount.m_config.stab_pitch);
		pG->v("stabRoll", &m_apMount.m_config.stab_roll);
		pG->v("stabYaw", &m_apMount.m_config.stab_yaw);
		pG->v("mountMode", &m_apMount.m_config.mount_mode);
	}

	string iName;

	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_AP_target_base", &iName);
	m_pAPtarget = (_AP_base*) (pK->root()->getChildInst(iName));

	return true;
}

bool _AP_goto::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: " << retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_goto::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMav,-1);

	return this->_AP_posCtrl::check();
}

void _AP_goto::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AP_posCtrl::update();
		updateGoto();

		this->autoFPSto();
	}
}

bool _AP_goto::updateGoto(void)
{
	IF_F(check() < 0);

	m_bTarget = findTarget();
	m_pAP->m_pMav->clDoSetRelay(m_iLEDpin, m_bTarget);

	IF_F(!bActive());

	if(m_apMount.m_bEnable)
		m_pAP->setMount(m_apMount);

	if(!m_bTarget)
	{
		releaseCtrl();
		return false;
	}

	setPosGlobal();
	return true;
}

bool _AP_goto::findTarget(void)
{
	IF_F(check()<0);
	NULL_F(m_pAPtarget);
	NULL_F(m_pAPtarget->m_pMav);
	IF_F(!m_pAPtarget->m_pMav->m_globalPositionINT.bReceiving(m_tStamp));

	vDouble4 vAPpos = m_pAPtarget->getGlobalPos();
	IF_F(vAPpos.x <= 0.0);
	IF_F(vAPpos.y <= 0.0);

	m_vTargetGlobal.x = vAPpos.x;
	m_vTargetGlobal.y = vAPpos.y;

	return true;
}

void _AP_goto::draw(void)
{
	this->_AP_posCtrl::draw();
}

}
