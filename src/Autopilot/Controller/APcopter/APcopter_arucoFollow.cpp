#include "APcopter_arucoFollow.h"

namespace kai
{

APcopter_arucoFollow::APcopter_arucoFollow()
{
	m_pAP = NULL;
	m_pArUco = NULL;
	m_pPC = NULL;

	m_vTarget.x = 0.5;
	m_vTarget.y = 0.5;
	m_vTarget.z = 10.0;
	m_vTarget.w = -1.0;

	m_vPos.x = 0.5;
	m_vPos.y = 0.5;
	m_vPos.z = 10.0;
	m_vPos.w = -1.0;

	m_tag = 0;
	m_angle = -1.0;
	m_bTarget = false;

}

APcopter_arucoFollow::~APcopter_arucoFollow()
{
}

bool APcopter_arucoFollow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,tag);
	KISSm(pK,angle);

	pK->v("x", &m_vTarget.x);
	pK->v("y", &m_vTarget.y);
	pK->v("z", &m_vTarget.z);
	pK->v("w", &m_vTarget.w);

	//link
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_ArUco", &iName);
	m_pArUco = (_ObjectBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pArUco, iName + ": not found");

	iName = "";
	pK->v("APcopter_posCtrlRC", &iName);
	m_pPC = (APcopter_posCtrlRC*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	return true;
}

int APcopter_arucoFollow::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pArUco,-1);
	NULL__(m_pArUco->m_pVision,-1);
	NULL__(m_pPC,-1);

	return 0;
}

void APcopter_arucoFollow::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!isActive())
	{
		m_pArUco->goSleep();
		return;
	}

	if(m_bStateChanged)
	{
		m_pArUco->wakeUp();
	}

	_VisionBase* pV = m_pArUco->m_pVision;
	vInt2 cSize;
	pV->info(&cSize, NULL, NULL);

	OBJECT* pO = newFound();
	if(!pO)
	{
		m_pAM->transit("CC_SEARCH");
		m_bTarget = false;
		return;
	}

	m_vPos.x = pO->m_bb.x/(double)cSize.x;
	m_vPos.y = pO->m_bb.y/(double)cSize.y;
	m_vPos.z = (double)m_pAP->m_pMavlink->m_msg.global_position_int.relative_alt * 0.001;
	m_vPos.w = pO->m_bb.w;	//angle

	m_bTarget = true;
	m_pAM->transit("CC_FOLLOW");
	m_pPC->setPos(m_vPos);
}

OBJECT* APcopter_arucoFollow::newFound(void)
{
	OBJECT* pO;
	int i=0;
	while((pO = m_pArUco->at(i++)) != NULL)
	{
		IF_CONT(pO->m_topClass != m_tag);

		return pO;
	}

	return NULL;
}

bool APcopter_arucoFollow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg = *this->getName() + ": ";

	if(!isActive())
	{
		msg += "Inactive";
	}
	else if (m_bTarget)
	{
		circle(*pMat, Point(m_vPos.x * pMat->cols,
							m_vPos.y * pMat->rows),
				pMat->cols * pMat->rows * 0.00005, Scalar(0, 0, 255), 2);

		msg += "Tag pos = (" + f2str(m_vPos.x) + ", "
							   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ", "
							   + f2str(m_vPos.w) + ")";
	}
	else
	{
		msg += "Tag not found";
	}

	pWin->addMsg(&msg);

	circle(*pMat, Point(m_vTarget.x * pMat->cols,
						m_vTarget.y * pMat->rows),
			pMat->cols * pMat->rows * 0.00005, Scalar(0, 255, 0), 2);

	return true;
}

bool APcopter_arucoFollow::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;
	if(!isActive())
	{
		msg = "Inactive";
	}
	else if (m_bTarget)
	{
		msg = "Tag pos = (" + f2str(m_vPos.x) + ", "
							   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ", "
							   + f2str(m_vPos.w) + ")";
	}
	else
	{
		msg = "Tag not found";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
