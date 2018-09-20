#include "APcopter_edgeHold.h"

namespace kai
{

APcopter_edgeHold::APcopter_edgeHold()
{
	m_pAP = NULL;
	m_pDE = NULL;
	m_pDB = NULL;
	m_pGPS = NULL;

	m_zTop = 50.0;
	m_vPos.init();
}

APcopter_edgeHold::~APcopter_edgeHold()
{
}

bool APcopter_edgeHold::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,zTop);

	int n = 3;
	pK->v("nMedian", &n);
	m_fX.init(n, 0);
	m_fY.init(n, 0);

	//link
	string iName;

	iName = "";
	F_ERROR_F(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	F_ERROR_F(pK->v("_DepthEdge", &iName));
	m_pDE = (_DepthEdge*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDE, iName + ": not found");

	iName = "";
	F_ERROR_F(pK->v("_DistSensorBase", &iName));
	m_pDB = (_DistSensorBase*)(pK->root()->getChildInst(iName));
	IF_Fl(!m_pDB, iName + ": not found");

	iName = "";
	F_ERROR_F(pK->v("_GPS", &iName));
	m_pGPS = (GPS*)(pK->root()->getChildInst(iName));
	IF_Fl(!m_pGPS, iName + ": not found");

	return true;
}

int APcopter_edgeHold::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pDE,-1);
	NULL__(m_pDB,-1);
	NULL__(m_pGPS,-1);

	return 0;
}

void APcopter_edgeHold::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!isActive());
	_DepthVisionBase* pDV = m_pDE->m_pDV;
	NULL_(pDV);

	vDouble4* pPos = m_pDE->pos();
	if(pPos->y > 0.0)
	{
		m_fY.input(pPos->y);
		m_vPos.y = m_pDE->m_rMax.y - m_fY.v();

		if(pPos->x > 0.0)
		{
			m_fX.input(pPos->x);
			m_vPos.x = - m_fY.v() * tan((m_fX.v() - 0.5) * pDV->m_fovW * DEG_RAD);
			//negative is needed according to the reference
		}

		m_pGPS->m_nSat = 10;
	}
	else
	{
		m_pGPS->m_nSat = 0;
	}

	double z = m_pDB->dAvr();
	if(z > 0.0)
		m_vPos.z = m_zTop - z;

	vDouble3 dPos;
	dPos.x = m_vPos.y; //Forward
	dPos.y = m_vPos.x; //Right
	dPos.z = m_vPos.z;

	m_pGPS->setRelPos(dPos);
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
				f2str(m_vPos.x) + ", " +
				f2str(m_vPos.y) + ", " +
				f2str(m_vPos.z) + ", " +
				f2str(m_vPos.w) + ")";
	}

	pWin->addMsg(&msg);

	return true;
}

bool APcopter_edgeHold::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;

	if(!isActive())
	{
		msg = "Inactive";
	}
	else
	{
		msg = "Edge pos = (" +
				f2str(m_vPos.x) + ", " +
				f2str(m_vPos.y) + ", " +
				f2str(m_vPos.z) + ", " +
				f2str(m_vPos.w) + ")";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
