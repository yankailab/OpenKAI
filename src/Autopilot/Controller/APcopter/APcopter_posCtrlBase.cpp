#include "APcopter_posCtrlBase.h"

namespace kai
{

APcopter_posCtrlBase::APcopter_posCtrlBase()
{
	m_pAP = NULL;
	m_vTarget.init();
	m_vPos.init();
}

APcopter_posCtrlBase::~APcopter_posCtrlBase()
{
}

bool APcopter_posCtrlBase::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	Kiss* pR;
	pR = pK->o("target");
	if(!pR->empty())
	{
		pR->v("x", &m_vTarget.x);
		pR->v("y", &m_vTarget.y);
		pR->v("z", &m_vTarget.z);
		pR->v("yaw", &m_vTarget.w);
	}

	//link
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

int APcopter_posCtrlBase::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->ActionBase::check();
}

void APcopter_posCtrlBase::setTargetPos(vDouble4& vT)
{
	m_vTarget = vT;
}

void APcopter_posCtrlBase::setPos(vDouble4& vP)
{
	m_vPos = vP;
}

void APcopter_posCtrlBase::clear(void)
{
}

bool APcopter_posCtrlBase::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg = *this->getName();
	pWin->addMsg(&msg);

	pWin->tabNext();

	if(!isActive())
	{
		msg = "Inactive";
		pWin->addMsg(&msg);
	}
	else
	{
		msg = "Pos = (" + f2str(m_vPos.x) + ", "
						 + f2str(m_vPos.y) + ", "
						 + f2str(m_vPos.z) + ", "
						 + f2str(m_vPos.w) + ")";
		pWin->addMsg(&msg);

		msg = "Target = (" + f2str(m_vTarget.x) + ", "
						    + f2str(m_vTarget.y) + ", "
						    + f2str(m_vTarget.z) + ", "
						    + f2str(m_vTarget.w) + ")";
		pWin->addMsg(&msg);
	}

	pWin->tabPrev();

	circle(*pMat, Point(m_vPos.x*pMat->cols, m_vPos.y*pMat->rows),
			pMat->cols*pMat->rows*0.00005,
			Scalar(0, 255, 0), 2);
	circle(*pMat, Point(m_vTarget.x*pMat->cols, m_vTarget.y*pMat->rows),
			pMat->cols*pMat->rows*0.00005,
			Scalar(0, 0, 255), 2);

	return true;
}

bool APcopter_posCtrlBase::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;

	if(!isActive())
	{
		msg = "Inactive";
		COL_MSG;
		iY++;
		mvaddstr(iY, CLI_X_MSG, msg.c_str());
	}
	else
	{
		msg = "Pos = (" + f2str(m_vPos.x) + ", "
						+ f2str(m_vPos.y) + ", "
						+ f2str(m_vPos.z) + ", "
						+ f2str(m_vPos.w) + ")";
		COL_MSG;
		iY++;
		mvaddstr(iY, CLI_X_MSG, msg.c_str());

		msg = "Target = (" + f2str(m_vTarget.x) + ", "
						   + f2str(m_vTarget.y) + ", "
						   + f2str(m_vTarget.z) + ", "
						   + f2str(m_vTarget.w) + ")";
		COL_MSG;
		iY++;
		mvaddstr(iY, CLI_X_MSG, msg.c_str());
	}

	return true;
}

}
