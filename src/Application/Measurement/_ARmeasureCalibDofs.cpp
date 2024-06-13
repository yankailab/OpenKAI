/*
 * _ARmeasureCalibDofs.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_ARmeasureCalibDofs.h"

namespace kai
{

	_ARmeasureCalibDofs::_ARmeasureCalibDofs()
	{
		m_pA = NULL;
		m_pR = NULL;
		m_pW = NULL;

		m_step = 0.01;
		m_drawCol = Scalar(0, 255, 0);
		m_pFt = NULL;
	}

	_ARmeasureCalibDofs::~_ARmeasureCalibDofs()
	{
	}

	bool _ARmeasureCalibDofs::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;

		n = "";
		pK->v("_ARmeasure", &n);
		m_pA = (_ARmeasure *)(pK->findModule(n));
		IF_Fl(!m_pA, n + " not found");

		n = "";
		pK->v("_Remap", &n);
		m_pR = (_Remap *)(pK->findModule(n));
		IF_Fl(!m_pR, n + " not found");

		n = "";
		pK->v("_WindowCV", &n);
		m_pW = (_WindowCV *)(pK->findModule(n));
		IF_Fl(!m_pW, n + " not found");

		return true;
	}

	bool _ARmeasureCalibDofs::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ARmeasureCalibDofs::check(void)
	{
		NULL__(m_pA, -1);
		NULL__(m_pW, -1);
		NULL__(m_pR, -1);
		IF__(m_pR->BGR()->bEmpty(), -1);

		return 0;
	}

	void _ARmeasureCalibDofs::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			this->_ModuleBase::update();
			updateCalib();

			m_pT->autoFPSto();
		}
	}

	void _ARmeasureCalibDofs::updateCalib(void)
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
	void _ARmeasureCalibDofs::sOnBtnClear(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->clear();
	}

	void _ARmeasureCalibDofs::sOnBtnSave(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->save();
	}

	void _ARmeasureCalibDofs::sOnBtnMode(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->mode(f);
	}

	void _ARmeasureCalibDofs::sOnBtnXi(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Xi();
	}

	void _ARmeasureCalibDofs::sOnBtnXd(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Xd();
	}

	void _ARmeasureCalibDofs::sOnBtnYi(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Yi();
	}

	void _ARmeasureCalibDofs::sOnBtnYd(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Yd();
	}

	void _ARmeasureCalibDofs::sOnBtnZi(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Zi();
	}

	void _ARmeasureCalibDofs::sOnBtnZd(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Zd();
	}

	void _ARmeasureCalibDofs::sOnBtnSi(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Si();
	}

	void _ARmeasureCalibDofs::sOnBtnSd(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Sd();
	}

	// UI handler
	void _ARmeasureCalibDofs::clear(void)
	{
		NULL_(m_pA);
		vFloat3 v;
		v.init(0);
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::save(void)
	{
		NULL_(m_pA);
		m_pA->saveCalib();
	}

	void _ARmeasureCalibDofs::mode(uint32_t f)
	{
		NULL_(m_pSC);

		if (f & 1) //long push
			m_pSC->transit("vertex");
		else
			m_pSC->transit("calibCam");
	}

	void _ARmeasureCalibDofs::Xi(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.x += m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::Xd(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.x -= m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::Yi(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.y += m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::Yd(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.y -= m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::Zi(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.z += m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::Zd(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.z -= m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::Si(void)
	{
		m_step *= 10.0;
	}

	void _ARmeasureCalibDofs::Sd(void)
	{
		m_step *= 0.1;
	}

	// UI draw
	void _ARmeasureCalibDofs::drawCalibData(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		int x = 30;
		int y = 50;
		int s = 20;
		int n = 4;
		int nd = 6;

		string c[4];
		vFloat3 vDofsP = m_pA->getDofsP();
		c[0] = "DofsPx: " + lf2str(vDofsP.x, nd);
		c[1] = "DofsPy: " + lf2str(vDofsP.y, nd);
		c[2] = "DofsPz: " + lf2str(vDofsP.z, nd);
		c[3] = "Step: " + lf2str(m_step, nd);

		for (int i = 0; i < n; i++)
		{
			m_pFt->putText(*pM, c[i],
						   Point(x, y+s*i),
						   20,
						   Scalar(0, 255, 0),
						   -1,
						   cv::LINE_AA,
						   false);
		}
	}

	void _ARmeasureCalibDofs::draw(void *pFrame)
	{
		NULL_(pFrame);
		IF_(!bActive());

		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		Frame *pF = (Frame*)pFrame;
		Mat *pMw = pF->m();
		IF_(pMw->empty());
		m_pFt = pWin->getFont();

		Mat mV;
		m_pR->BGR()->m()->copyTo(mV);

		Rect r;
		r.x = 0;
		r.y = 0;
		r.width = mV.cols;
		r.height = mV.rows;
		mV.copyTo((*pMw)(r));

		drawCalibData(pMw);
	}

}
