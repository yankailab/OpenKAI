#include "_APcopter_line.h"

namespace kai
{

_APcopter_line::_APcopter_line()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pDet = NULL;
	m_pDS = NULL;

	m_bFound = false;
	m_tO.init();

	m_vSetP.x = 0.5;	//roll in screen scale
	m_vSetP.y = 6.0;	//pitch in m
	m_vSetP.z = 0.5;	//alt in screen scale
	m_vSetP.w = 0.0;	//north
	m_vTargetP = m_vSetP;

	m_bAltDir = false;
	m_apMount.init();
}

_APcopter_line::~_APcopter_line()
{
}

bool _APcopter_line::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("bAltDir",&m_bAltDir);

	int n;
	pK->v("nMed",&n);
	m_fMed.init(n,0);

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

	pG = pK->o("targetPos");
	if(!pG->empty())
	{
		pG->v("x", &m_vTargetP.x);
		pG->v("y", &m_vTargetP.y);
		pG->v("z", &m_vTargetP.z);
		pG->v("w", &m_vTargetP.w);
	}

	pG = pK->o("myPos");
	if(!pG->empty())
	{
		pG->v("x", &m_vSetP.x);
		pG->v("y", &m_vSetP.y);
		pG->v("z", &m_vSetP.z);
		pG->v("w", &m_vSetP.w);
	}

	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (_APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pDet = (_Line*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDet, iName + ": not found");

	iName = "";
	pK->v("APcopter_posCtrl", &iName);
	m_pPC = (_APcopter_posCtrl*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	iName = "";
	pK->v("_DistSensorBase", &iName);
	m_pDS = (_DistSensorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDS, iName + ": not found");

	return true;
}

int _APcopter_line::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pPC,-1);
	NULL__(m_pDet,-1);
	NULL__(m_pDS,-1);
	_VisionBase* pV = m_pDet->m_pVision;
	NULL__(pV,-1);

	return this->_AutopilotBase::check();
}

void _APcopter_line::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		m_vTargetP = m_vSetP;
		m_pPC->setEnable(false);
		return;
	}

	m_pAP->setMount(m_apMount);

//	if(m_pDet->m_bTower)
//	{
//		m_bFound = true;
//		m_vTargetPos = m_vMyPos;
//		m_vTargetPos.z += 0.1;
//		m_pPC->setPos(m_vMyPos, m_vTargetPos);
//		m_pPC->setON(true);
//		return;
//	}

	if(!find())
	{
		m_bFound = false;
		m_vTargetP = m_vSetP;
		m_pPC->setEnable(false);
		return;
	}

	m_bFound = true;
//	m_pPC->setPos(m_vSetP, m_vTargetP);
	m_pPC->setEnable(true);
}

bool _APcopter_line::find(void)
{
	IF__(check()<0, false);

	//alt
	OBJECT* tO = NULL;
	OBJECT* pO;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
		tO = pO;
		break;		//TODO: multiple lines
	}

	if(tO)
	{
		m_fMed.input((double)tO->m_bb.midX());
		m_vTargetP.z = m_fMed.v();
		if(m_bAltDir)m_vTargetP.z = 1.0 - m_vTargetP.z;
	}
	else
	{
		m_vTargetP.z = m_vSetP.z;
	}

	//pitch
	double ds = m_pDS->dMin();
	if(ds > 0.0)
		m_vTargetP.y = ds;
	else
		m_vTargetP.y = m_vSetP.y;

	return true;
}

bool _APcopter_line::bFound(void)
{
	return m_bFound;
}

void _APcopter_line::draw(void)
{
	this->_AutopilotBase::draw();

	if(!bActive())
		addMsg("Inactive",1);

	addMsg("Target = (" + f2str(m_vTargetP.x) + ", "
							   + f2str(m_vTargetP.y) + ", "
					           + f2str(m_vTargetP.z) + ", "
				           	   + f2str(m_vTargetP.w) + ")",1);

	addMsg("MyPos = (" + f2str(m_vSetP.x) + ", "
							   + f2str(m_vSetP.y) + ", "
					           + f2str(m_vSetP.z) + ", "
				           	   + f2str(m_vSetP.w) + ")",1);
}

}
