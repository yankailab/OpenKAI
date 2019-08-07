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
		Rect r;
		r.x = o.m_bb.x * m.cols;
		r.y = o.m_bb.y * m.rows;
		r.width = o.m_bb.z * m.cols - r.x;
		r.height = o.m_bb.w * m.rows - r.y;

		string ag = "";

		m_pG->classify(m(r),&o);
		ag += m_pG->m_vClass[o.m_topClass].m_name;

		m_pA->classify(m(r),&o);
		ag += ", " + m_pA->m_vClass[o.m_topClass].m_name;

		strncpy(o.m_pText, ag.c_str(), ag.length());

		this->add(&o);
	}

	return true;
}

bool _ShopCam::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Scalar col = Scalar(0,255,0);

//	Rect r = convertBB<vInt4>(convertBB(m_vRoi, cs));
//	rectangle(*pMat, r, col, 3);

	return true;
}

bool _ShopCam::console(int& iY)
{
	IF_F(!this->_DetectorBase::console(iY));

	string msg;

	return true;
}

}
