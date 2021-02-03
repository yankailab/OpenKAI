/*
 * _ShopCam.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_ShopCam.h"

#ifdef USE_OPENCV

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

bool _ShopCam::init(void *pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	string n;

	n = "";
	F_ERROR_F(pK->v("_DetectorBase", &n));
	m_pD = (_DetectorBase*) (pK->getInst(n));
	IF_Fl(!m_pD, n + " not found");

	n = "";
	F_ERROR_F(pK->v("_DNNclassifierG", &n));
	m_pG = (_DNNclassifier*) (pK->getInst(n));
	IF_Fl(!m_pG, n + " not found");

	n = "";
	F_ERROR_F(pK->v("_DNNclassifierA", &n));
	m_pA = (_DNNclassifier*) (pK->getInst(n));
	IF_Fl(!m_pA, n + " not found");

	return true;
}

bool _ShopCam::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _ShopCam::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		if (check() >= 0)
		{
			updateDet();
			m_pU->updateObj();

			if (m_bGoSleep)
				m_pU->m_pPrev->clear();
		}

		m_pT->autoFPSto();
	}
}

int _ShopCam::check(void)
{
	NULL__(m_pU, -1);

	NULL__(m_pD, -1);
	NULL__(m_pD->m_pV, -1);
	IF__(m_pD->m_pV->BGR()->bEmpty(), -1);

	NULL__(m_pG, -1);
	NULL__(m_pA, -1);

	return 0;
}

bool _ShopCam::updateDet(void)
{
	m_fBGR.copy(*m_pD->m_pV->BGR());
	Mat m = *m_fBGR.m();

	_Object *pO;
	int i = 0;
	while ((pO = m_pD->m_pU->get(i++)) != NULL)
	{
		_Object o = *pO;
		Rect r = bb2Rect(o.getBB2Dscaled(m.cols, m.rows));

		string ag = "";

		m_pA->classify(m(r), &o);
		ag += m_pA->m_vClass[o.getTopClass()].m_name;

		m_pG->classify(m(r), &o);
		ag = m_pG->m_vClass[o.getTopClass()].m_name + ", " + ag;

		char pTxt[128];
		strncpy(pTxt, ag.c_str(), ag.length());
		string t = pTxt;
		o.setText(t);

		_Object *pP;
		_Object *pCorres = NULL;
		float minD = 10.0;
		int j = 0;
		while ((pP = m_pU->m_pPrev->get(j++)) != NULL)
		{
			float dX = pP->getX() - o.getX();
			float dY = pP->getY() - o.getY();
			float d = dX + dY;
			IF_CONT(d > minD);

			pCorres = pP;
			minD = d;
		}

		if (pCorres)
		{
//			for(int t=0; t<OBJ_N_TRAJ; t++)
//				o.m_pTraj[t] = pCorres->m_pTraj[t];
//
//			o.m_iTraj = pCorres->m_iTraj;
//			o.m_nTraj = pCorres->m_nTraj;
//
//			o.addTrajectory(o.m_bb.center());
		}

		m_pU->add(o);
	}

	return true;
}

void _ShopCam::draw(void)
{
	this->_ModuleBase::draw();

	IF_(!checkWindow());

	Window *pWin = (Window*) this->m_pWindow;
	Frame *pFrame = pWin->getFrame();
	Mat *pM = pFrame->m();

	Scalar oCol;
	Scalar mCol = Scalar(255, 200, 200);
	Scalar fCol = Scalar(200, 200, 255);

	int nM = 0;
	int nF = 0;

	_Object *pO;
	int i = 0;
	while ((pO = m_pU->get(i++)) != NULL)
	{
		int iClass = pO->getTopClass();

		if (iClass == 0)
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
		Rect r = bb2Rect(pO->getBB2Dscaled(pM->cols, pM->rows));
		rectangle(*pM, r, oCol, 2);

		//text
		if (m_bDrawText)
		{
			string oName = pO->getText();
			if (oName.length() > 0)
			{
				putText(*pM, oName, Point(r.x + 15, r.y + 50),
						FONT_HERSHEY_SIMPLEX, 0.8, oCol, 2);
			}
		}

		//trajectory
//		if (pO->m_nTraj > 2)
//		{
//			for (int t = 0; t < pO->m_nTraj; t++)
//			{
//				int iFrom = pO->m_iTraj - t - 1;
//				if (iFrom < 0)
//					iFrom = OBJ_N_TRAJ - t - 1;
//
//				int iTo = iFrom - 1;
//				if (iTo < 0)
//					iTo = OBJ_N_TRAJ - 1;
//
//				Point pF = Point(pO->m_pTraj[iFrom].x * pMat->cols,
//						pO->m_pTraj[iFrom].y * pMat->rows);
//				Point pT = Point(pO->m_pTraj[iTo].x * pMat->cols,
//						pO->m_pTraj[iTo].y * pMat->rows);
//
//				line(*pMat, pF, pT, Scalar(200, 200, 0), 2);
//			}
//		}
	}

	putText(*pM, "Male: " + i2str(nM) + ", Female: " + i2str(nF),
			Point(m_classLegendPos.x,
					m_classLegendPos.y + i * m_classLegendPos.z),
			FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 0), 2);

}

}
#endif
