#include "APcopter_edgeHold.h"

namespace kai
{

APcopter_edgeHold::APcopter_edgeHold()
{
	m_pAP = NULL;
	m_pDV = NULL;
	m_pPC = NULL;

	m_vTarget.x = 0.5;
	m_vTarget.y = 0.5;
	m_vTarget.z = 10.0;
	m_vTarget.w = 0.0;

	m_vPos.x = 0.5;
	m_vPos.y = 0.5;
	m_vPos.z = 10.0;
	m_vPos.w = 0.0;

	m_edgeTop = 0.0;
	m_edgeBottom = 0.0;

	m_wD = 320;
	m_hD = 200;

}

APcopter_edgeHold::~APcopter_edgeHold()
{
}

bool APcopter_edgeHold::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,wD);
	KISSm(pK,hD);

	pK->v("x", &m_vTarget.x);
	pK->v("y", &m_vTarget.y);
	pK->v("z", &m_vTarget.z);
	pK->v("w", &m_vTarget.w);

	int n = 3;
	pK->v("nMedian", &n);
	m_fMed.init(n, 0);

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
	F_INFO(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDV, iName + ": not found");

	m_pf.init(m_hD,2);

	return true;
}

int APcopter_edgeHold::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pDV,-1);

	return 0;
}

void APcopter_edgeHold::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!isActive());

	double iEdge = detectEdge();
	IF_(iEdge < 0.0);
	m_fMed.input(iEdge);

}

double APcopter_edgeHold::detectEdge(void)
{
	Frame* pfDepth = m_pDV->Depth();
	IF__(pfDepth->bEmpty(), -1);

	Frame fD = pfDepth->resize(m_wD, m_hD);
	Mat mF;
	cv::threshold(*fD.m(), mF, 2.0, 30.0, THRESH_BINARY);//TODO

	int i,j;
	for(i=0; i<mF.rows; i++)
	{
		double dEdge = 0;
		double iEdge = 0;

		for(j=0; j<mF.cols-1; j++)
		{
			double d = abs((double)mF.at<float>(i,j+1) - (double)mF.at<float>(i,j));
			IF_CONT(d < dEdge);

			dEdge = d;
			iEdge = j;
		}

		m_pf.x(i, iEdge);
	}

	m_pf.fit();
	double ovW = 1.0/(double)mF.cols;
	m_edgeTop = m_pf.yPoly(0) * ovW;
	m_edgeBottom = m_pf.yPoly(mF.rows-1) * ovW;

	return (m_edgeTop + m_edgeBottom)*0.5;
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
							m_vPos.y * pMat->rows),
				pMat->cols * pMat->rows * 0.00005, Scalar(0, 0, 255), 2);

		msg += "Edge top = " + f2str(m_edgeTop) + ", bottom = " + f2str(m_edgeBottom);
	}

	pWin->addMsg(&msg);

	line(*pMat,
		 Point(m_edgeTop * pMat->cols, 0),
		 Point(m_edgeBottom * pMat->cols, pMat->rows),
		 Scalar(0,255,0), 6);

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
		msg += "Edge top = " + f2str(m_edgeTop) + ", bottom = " + f2str(m_edgeBottom);
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
