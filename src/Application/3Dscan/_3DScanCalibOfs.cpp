/*
 * _3DScanCalibOfs.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_3DScanCalibOfs.h"

namespace kai
{

	_3DScanCalibOfs::_3DScanCalibOfs()
	{
		m_pV = NULL;
		m_pW = NULL;

		m_step = 0.01;
		m_drawCol = Scalar(0, 255, 0);
		m_pFt = NULL;
	}

	_3DScanCalibOfs::~_3DScanCalibOfs()
	{
	}

	bool _3DScanCalibOfs::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + " not found");

		n = "";
		pK->v("_WindowCV", &n);
		m_pW = (_WindowCV *)(pK->getInst(n));
		IF_Fl(!m_pW, n + " not found");

		return true;
	}

	bool _3DScanCalibOfs::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _3DScanCalibOfs::check(void)
	{
		NULL__(m_pW, -1);
		NULL__(m_pV, -1);
		IF__(m_pV->BGR()->bEmpty(), -1);

		return 0;
	}

	void _3DScanCalibOfs::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			this->_StateBase::update();
			updateCalib();

			m_pT->autoFPSto();
		}
	}

	void _3DScanCalibOfs::updateCalib(void)
	{
		IF_(!bActive());
		if (bStateChanged())
		{
			m_pW->setBtnVisibleAll(false);
			m_pW->setCbBtn("Save", sOnBtnSave, this);
			m_pW->setCbBtn("Clear", sOnBtnClear, this);
			m_pW->setCbBtn("Mode", sOnBtnMode, this);
			m_pW->setBtnVisible("Save", true);
			m_pW->setBtnVisible("Clear", true);
			m_pW->setBtnVisible("Mode", true);

			m_pW->setBtnLabel("Mode", "CD");

			m_pW->setCbBtn("Xi", sOnBtnXi, this);
			m_pW->setCbBtn("Xd", sOnBtnXd, this);
			m_pW->setCbBtn("Yi", sOnBtnYi, this);
			m_pW->setCbBtn("Yd", sOnBtnYd, this);
			m_pW->setCbBtn("Zi", sOnBtnZi, this);
			m_pW->setCbBtn("Zd", sOnBtnZd, this);
			m_pW->setCbBtn("Si", sOnBtnSi, this);
			m_pW->setCbBtn("Sd", sOnBtnSd, this);
			m_pW->setBtnVisible("Xi", true);
			m_pW->setBtnVisible("Xd", true);
			m_pW->setBtnVisible("Yi", true);
			m_pW->setBtnVisible("Yd", true);
			m_pW->setBtnVisible("Zi", true);
			m_pW->setBtnVisible("Zd", true);
			m_pW->setBtnVisible("Si", true);
			m_pW->setBtnVisible("Sd", true);
		}
	}

	// callbacks
	void _3DScanCalibOfs::sOnBtnClear(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->clear();
	}

	void _3DScanCalibOfs::sOnBtnSave(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->save();
	}

	void _3DScanCalibOfs::sOnBtnMode(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->mode(f);
	}

	void _3DScanCalibOfs::sOnBtnXi(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->Xi();
	}

	void _3DScanCalibOfs::sOnBtnXd(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->Xd();
	}

	void _3DScanCalibOfs::sOnBtnYi(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->Yi();
	}

	void _3DScanCalibOfs::sOnBtnYd(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->Yd();
	}

	void _3DScanCalibOfs::sOnBtnZi(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->Zi();
	}

	void _3DScanCalibOfs::sOnBtnZd(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->Zd();
	}

	void _3DScanCalibOfs::sOnBtnSi(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->Si();
	}

	void _3DScanCalibOfs::sOnBtnSd(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibOfs *)pInst)->Sd();
	}

	// UI handler
	void _3DScanCalibOfs::clear(void)
	{
		vFloat3 v;
		v.init(0);
//		m_pA->setDofsP(v);
	}

	void _3DScanCalibOfs::save(void)
	{
//		NULL_(m_pA);
//		m_pA->saveCalib();
	}

	void _3DScanCalibOfs::mode(uint32_t f)
	{
		NULL_(m_pSC);

		if (f & 1) //long push
			m_pSC->transit("vertex");
		else
			m_pSC->transit("calibCam");
	}

	void _3DScanCalibOfs::Xi(void)
	{
//		NULL_(m_pA);
//		vFloat3 v = m_pA->getDofsP();
//		v.x += m_step;
//		m_pA->setDofsP(v);
	}

	void _3DScanCalibOfs::Xd(void)
	{
//		NULL_(m_pA);
//		vFloat3 v = m_pA->getDofsP();
//		v.x -= m_step;
//		m_pA->setDofsP(v);
	}

	void _3DScanCalibOfs::Yi(void)
	{
//		NULL_(m_pA);
//		vFloat3 v = m_pA->getDofsP();
//		v.y += m_step;
//		m_pA->setDofsP(v);
	}

	void _3DScanCalibOfs::Yd(void)
	{
		// NULL_(m_pA);
		// vFloat3 v = m_pA->getDofsP();
		// v.y -= m_step;
		// m_pA->setDofsP(v);
	}

	void _3DScanCalibOfs::Zi(void)
	{
		// NULL_(m_pA);
		// vFloat3 v = m_pA->getDofsP();
		// v.z += m_step;
		// m_pA->setDofsP(v);
	}

	void _3DScanCalibOfs::Zd(void)
	{
		// NULL_(m_pA);
		// vFloat3 v = m_pA->getDofsP();
		// v.z -= m_step;
		// m_pA->setDofsP(v);
	}

	void _3DScanCalibOfs::Si(void)
	{
		m_step *= 10.0;
	}

	void _3DScanCalibOfs::Sd(void)
	{
		m_step *= 0.1;
	}

	// UI draw
	void _3DScanCalibOfs::drawCalibData(Mat *pM)
	{
		// NULL_(pM);
		// NULL_(m_pFt);

		// int x = 30;
		// int y = 50;
		// int s = 20;
		// int n = 4;
		// int nd = 6;

		// string c[4];
		// vFloat3 vDofsP = m_pA->getDofsP();
		// c[0] = "DofsPx: " + lf2str(vDofsP.x, nd);
		// c[1] = "DofsPy: " + lf2str(vDofsP.y, nd);
		// c[2] = "DofsPz: " + lf2str(vDofsP.z, nd);
		// c[3] = "Step: " + lf2str(m_step, nd);

		// for (int i = 0; i < n; i++)
		// {
		// 	m_pFt->putText(*pM, c[i],
		// 				   Point(x, y+s*i),
		// 				   20,
		// 				   Scalar(0, 255, 0),
		// 				   -1,
		// 				   cv::LINE_AA,
		// 				   false);
		// }
	}

	void _3DScanCalibOfs::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		IF_(!bActive());

		this->_StateBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Mat *pMw = pWin->getNextFrame()->m();
		IF_(pMw->empty());
		m_pFt = pWin->getFont();

		Mat mV;
		m_pV->BGR()->m()->copyTo(mV);

		Rect r;
		r.x = 0;
		r.y = 0;
		r.width = mV.cols;
		r.height = mV.rows;
		mV.copyTo((*pMw)(r));

		drawCalibData(pMw);
	}

}
