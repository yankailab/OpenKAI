/*
 * _ShopCam.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_ShopCam.h"

namespace kai
{

_ShopCam::_ShopCam()
{
	m_pD = NULL;
	m_pG = NULL;
	m_pA = NULL;

}

_ShopCam::~_ShopCam()
{
}

bool _ShopCam::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pD = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pD, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_DNNclassifierG", &iName));
	m_pG = (_DNNclassifier*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pG, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_DNNclassifierA", &iName));
	m_pA = (_DNNclassifier*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pA, iName + " not found");

	return true;
}

bool _ShopCam::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _ShopCam::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateDet();

		updateObj();

		if (m_bGoSleep)
		{
			m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _ShopCam::check(void)
{
	NULL__(m_pD, -1);
	NULL__(m_pD->m_pVision, -1);
	IF__(m_pD->m_pVision->BGR()->bEmpty(), -1);

	NULL__(m_pG, -1);
	NULL__(m_pA, -1);

	return 0;
}

bool _ShopCam::updateDet(void)
{
	IF_F(check() < 0);
	m_fBGR.copy(*m_pD->m_pVision->BGR());
	Mat m = *m_fBGR.m();

	OBJECT* pO;
	int i = 0;
	while ((pO = m_pD->at(i++)) != NULL)
	{
		OBJECT o = *pO;
		vFloat4 bb = o.m_bb;
		Rect r;
		r.x = bb.x * m.cols;
		r.y = bb.y * m.rows;
		r.width = bb.z * m.cols - r.x;
		r.height = bb.w * m.rows - r.y;

		string ag = "";

		m_pA->classify(m(r),&o);
		ag += m_pA->m_vClass[o.m_topClass].m_name;

		m_pG->classify(m(r),&o);
		ag = m_pG->m_vClass[o.m_topClass].m_name + ", " + ag;

		strncpy(o.m_pText, ag.c_str(), ag.length());

		OBJECT* pP;
		OBJECT* pCorres = NULL;
		float minD = 10.0;
		int j = 0;
		while ((pP = m_pPrev->at(j++)) != NULL)
		{
			float dX = pP->m_bb.center().x - o.m_bb.center().x;
			float dY = pP->m_bb.center().y - o.m_bb.center().y;
			float d = dX + dY;
			IF_CONT(d > minD);

			pCorres = pP;
			minD = d;
		}

		if(pCorres)
		{
			for(int t=0; t<OBJ_N_TRAJ; t++)
				o.m_pTraj[t] = pCorres->m_pTraj[t];

			o.m_iTraj = pCorres->m_iTraj;
			o.m_nTraj = pCorres->m_nTraj;

			o.addTrajectory(o.m_bb.center());
		}

		this->add(&o);
	}

	return true;
}

bool _ShopCam::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;

	Scalar oCol;
	Scalar mCol = Scalar(255,200,200);
	Scalar fCol = Scalar(200,200,255);

	int nM = 0;
	int nF = 0;

	OBJECT* pO;
	int i=0;
	while((pO = at(i++)) != NULL)
	{
		int iClass = pO->m_topClass;

		if(pO->m_topClass == 0)
		{
			oCol = mCol;
			nM++;
		}
		else
		{
			oCol = fCol;
			nF++;
		}

		//bb
		Rect r = convertBB<vInt4>(convertBB(pO->m_bb, cs));
		rectangle(*pMat, r, oCol, 2);

		//text
		if(m_bDrawText)
		{
			string oName = string(pO->m_pText);
			if (oName.length()>0)
			{
				putText(*pMat, oName,
						Point(r.x + 15, r.y + 50),
						FONT_HERSHEY_SIMPLEX, 0.8, oCol, 2);
			}
		}

		//trajectory
		if(pO->m_nTraj > 2)
		{
			for(int t=0; t<pO->m_nTraj; t++)
			{
				int iFrom = pO->m_iTraj - t - 1;
				if(iFrom < 0)iFrom = OBJ_N_TRAJ - t - 1;

				int iTo = iFrom - 1;
				if(iTo < 0)iTo = OBJ_N_TRAJ - 1;

				Point pF = Point(pO->m_pTraj[iFrom].x * pMat->cols,
								 pO->m_pTraj[iFrom].y * pMat->rows);
				Point pT = Point(pO->m_pTraj[iTo].x * pMat->cols,
								 pO->m_pTraj[iTo].y * pMat->rows);

				line(*pMat, pF, pT, Scalar(200, 200, 0), 2);
			}
		}
	}

	putText(*pMat, "Male: " + i2str(nM) + ", Female: " + i2str(nF),
			Point(m_classLegendPos.x, m_classLegendPos.y + i*m_classLegendPos.z),
			FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0,255,0), 2);

	return true;

}

bool _ShopCam::console(int& iY)
{
	IF_F(!this->_DetectorBase::console(iY));

	string msg;

	return true;
}

}
