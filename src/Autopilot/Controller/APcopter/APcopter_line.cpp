#include "APcopter_line.h"

namespace kai
{

APcopter_line::APcopter_line()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pDet = NULL;
	m_pDS = NULL;

	m_bFound = false;
	m_tO.init();

	m_vMyPos.x = 0.5;	//roll in screen scale
	m_vMyPos.y = 6.0;	//pitch in m
	m_vMyPos.z = 0.5;	//alt in screen scale
	m_vMyPos.w = 0.0;	//north
	m_vTargetPos = m_vMyPos;

	m_bAltDir = false;
	m_rollSpd = 1.0;
	m_apMount.init();
}

APcopter_line::~APcopter_line()
{
}

bool APcopter_line::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, rollSpd);
	KISSm(pK, bAltDir);

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
		pG->v("x", &m_vTargetPos.x);
		pG->v("y", &m_vTargetPos.y);
		pG->v("z", &m_vTargetPos.z);
		pG->v("w", &m_vTargetPos.w);
	}

	pG = pK->o("myPos");
	if(!pG->empty())
	{
		pG->v("x", &m_vMyPos.x);
		pG->v("y", &m_vMyPos.y);
		pG->v("z", &m_vMyPos.z);
		pG->v("w", &m_vMyPos.w);
	}

	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pDet = (_Line*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDet, iName + ": not found");

	iName = "";
	pK->v("APcopter_posCtrl", &iName);
	m_pPC = (APcopter_posCtrl*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	iName = "";
	pK->v("_DistSensorBase", &iName);
	m_pDS = (_DistSensorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDS, iName + ": not found");

	return true;
}

int APcopter_line::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pPC,-1);
	NULL__(m_pDet,-1);
	NULL__(m_pDS,-1);
	_VisionBase* pV = m_pDet->m_pVision;
	NULL__(pV,-1);

	return this->ActionBase::check();
}

void APcopter_line::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		m_vTargetPos = m_vMyPos;
		m_pPC->setON(false);
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
		m_vTargetPos = m_vMyPos;
		m_pPC->setON(false);
		m_pPC->releaseCtrl();
		return;
	}

	m_bFound = true;
	m_pPC->setPos(m_vMyPos, m_vTargetPos);
	m_pPC->setON(true);
}

bool APcopter_line::find(void)
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
		m_vTargetPos.z = m_fMed.v();
		if(m_bAltDir)m_vTargetPos.z = 1.0 - m_vTargetPos.z;
	}
	else
	{
		m_vTargetPos.z = m_vMyPos.z;
	}

	//pitch
	double ds = m_pDS->dMin();
	if(ds > 0.0)
		m_vTargetPos.y = ds;
	else
		m_vTargetPos.y = m_vMyPos.y;

	//roll
	m_vTargetPos.x = m_vMyPos.x + m_rollSpd;

	return true;
}

bool APcopter_line::bFound(void)
{
	return m_bFound;
}

bool APcopter_line::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	pWin->tabNext();

	if(!bActive())
		pWin->addMsg("Inactive");

	pWin->addMsg("Target = (" + f2str(m_vTargetPos.x) + ", "
							   + f2str(m_vTargetPos.y) + ", "
					           + f2str(m_vTargetPos.z) + ", "
				           	   + f2str(m_vTargetPos.w) + ")");

	pWin->addMsg("MyPos = (" + f2str(m_vMyPos.x) + ", "
							   + f2str(m_vMyPos.y) + ", "
					           + f2str(m_vMyPos.z) + ", "
				           	   + f2str(m_vMyPos.w) + ")");

	pWin->tabPrev();

	return true;
}

bool APcopter_line::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;

	if(!bActive())
	{
		C_MSG("Inactive");
	}

	C_MSG("Target = (" + f2str(m_vTargetPos.x) + ", "
				     	 + f2str(m_vTargetPos.y) + ", "
						 + f2str(m_vTargetPos.z) + ", "
						 + f2str(m_vTargetPos.w) + ")");

	C_MSG("MyPos = (" + f2str(m_vMyPos.x) + ", "
				     	 + f2str(m_vMyPos.y) + ", "
						 + f2str(m_vMyPos.z) + ", "
						 + f2str(m_vMyPos.w) + ")");

	return true;
}

}
