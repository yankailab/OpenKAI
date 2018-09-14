#include "APcopter_edgeHold.h"

namespace kai
{

APcopter_edgeHold::APcopter_edgeHold()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pDE = NULL;

	m_vTarget.x = 0.5;
	m_vTarget.y = 0.5;
	m_vTarget.z = 10.0;
	m_vTarget.w = 0.0;

	m_vPos.init();
}

APcopter_edgeHold::~APcopter_edgeHold()
{
}

bool APcopter_edgeHold::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	int n = 3;
	pK->v("nMedian", &n);

	m_fX.init(n, 0);
	m_fY.init(n, 0);
	m_fZ.init(n, 0);

	//link
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("APcopter_posCtrlRC", &iName);
	m_pPC = (APcopter_posCtrlRC*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	iName = "";
	F_INFO(pK->v("_DepthEdge", &iName));
	m_pDE = (_DepthEdge*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDE, iName + ": not found");

	return true;
}

int APcopter_edgeHold::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pDE,-1);

	return 0;
}

void APcopter_edgeHold::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!isActive());

	vDouble4* pPos = m_pDE->pos();
	NULL_(pPos);

	m_fX.input(pPos->x);
	m_fY.input(pPos->y);
	m_fZ.input(pPos->z);

	m_vPos.x = m_fX.v();
	m_vPos.y = m_fY.v();
	m_vPos.z = m_fZ.v();

}

bool APcopter_edgeHold::draw(void)
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
	else
	{
		circle(*pMat, Point(m_vPos.x * pMat->cols,
							0.5 * pMat->rows),
							pMat->cols * pMat->rows * 0.00002, Scalar(0, 0, 255), 2);

		msg += "Edge pos = (" +
				f2str(m_vPos.x) +
				f2str(m_vPos.y) +
				f2str(m_vPos.z) +
				f2str(m_vPos.w) + ")";
	}

	pWin->addMsg(&msg);

	return true;
}

bool APcopter_edgeHold::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg = *this->getName() + ": ";

	if(!isActive())
	{
		msg += "Inactive";
	}
	else
	{
		msg += "Edge pos = (" +
				f2str(m_vPos.x) +
				f2str(m_vPos.y) +
				f2str(m_vPos.z) +
				f2str(m_vPos.w) + ")";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
