#include "APcopter_tracker.h"

namespace kai
{

APcopter_tracker::APcopter_tracker()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pT = NULL;

	m_apMode = -1;
	m_tBB.x = 0.4;
	m_tBB.y = 0.4;
	m_tBB.z = 0.6;
	m_tBB.w = 0.6;

	m_vMyPos.init();
	m_vMyPos.x = 0.5;
	m_vMyPos.y = 0.5;
	m_vTargetPos.init();
	m_apMount.init();

}

APcopter_tracker::~APcopter_tracker()
{
}

bool APcopter_tracker::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("apMode",&m_apMode);

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

	pG = pK->o("tBB");
	if(!pG->empty())
	{
		pG->v("x", &m_tBB.x);
		pG->v("y", &m_tBB.y);
		pG->v("z", &m_tBB.z);
		pG->v("w", &m_tBB.w);
	}

	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("APcopter_posCtrl", &iName);
	m_pPC = (APcopter_posCtrl*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	iName = "";
	pK->v("_TrackerBase", &iName);
	m_pT = (_TrackerBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pT, iName + ": not found");

	return true;
}

int APcopter_tracker::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pPC,-1);
	NULL__(m_pT,-1);

	return this->ActionBase::check();
}

void APcopter_tracker::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

//	static uint64_t tTrack = 0;
//	if(m_tStamp - tTrack > USEC_1SEC*5)
//	{
//		m_pT->startTrack(m_tBB);
//		tTrack = m_tStamp;
//	}
//	return;

	if(!bActive() || m_pAP->getApMode() != m_apMode)
	{
		m_vTargetPos = m_vMyPos;
		m_pPC->setPos(m_vMyPos, m_vMyPos);
		m_pPC->setON(false);
		m_pPC->releaseCtrl();
		m_pT->stopTrack();

		return;
	}

	m_pAP->setMount(m_apMount);

	if(m_bMissionChanged)//m_pAP->bApModeChanged())
	{
		m_vMyPos.w = m_pAP->m_apHdg;
		m_vTargetPos.w = m_pAP->m_apHdg;
		m_pT->startTrack(m_tBB);
	}

	vFloat4 bb = *m_pT->getBB();
	m_vTargetPos.x = bb.midX();
	m_vTargetPos.y = bb.midY();
	m_pPC->setON(true);
	m_pPC->setPos(m_vMyPos, m_vTargetPos);
}

bool APcopter_tracker::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	pWin->addMsg(*this->getName());

	pWin->tabNext();
	if(!bActive())
		pWin->addMsg("Inactive");

	pWin->addMsg("Target = (" + f2str(m_vTargetPos.x) + ", "
							   + f2str(m_vTargetPos.y) + ", "
					           + f2str(m_vTargetPos.z) + ", "
				           	   + f2str(m_vTargetPos.w) + ")");
	pWin->tabPrev();

	//bb
	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Rect r = convertBB<vInt4>(convertBB(m_tBB, cs));
	rectangle(*pMat, r, Scalar(255,255,0), 1);

	return true;
}

bool APcopter_tracker::console(int& iY)
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

	return true;
}

}
