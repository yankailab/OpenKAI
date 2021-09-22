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
		m_pV = NULL;
		m_pW = NULL;

		m_fKiss = "";
		m_step = 0.01;

		m_drawMsg = "";
		m_drawCol = Scalar(0, 255, 0);
		m_pFt = NULL;
	}

	_ARmeasureCalibDofs::~_ARmeasureCalibDofs()
	{
	}

	bool _ARmeasureCalibDofs::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("fKiss", &m_fKiss);

		string n;

		n = "";
		pK->v("_ARmeasure", &n);
		m_pA = (_ARmeasure *)(pK->getInst(n));
		IF_Fl(!m_pA, n + " not found");

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

	bool _ARmeasureCalibDofs::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ARmeasureCalibDofs::check(void)
	{
		NULL__(m_pA, -1);
		NULL__(m_pW, -1);
		NULL__(m_pV, -1);
		IF__(m_pV->BGR()->bEmpty(), -1);

		return 0;
	}

	void _ARmeasureCalibDofs::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			this->_StateBase::update();
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
			m_pW->setBtnLabel("Mode", "CD");
			m_pW->setBtnVisible("Mode", true);

			m_pW->setCbBtn("L", sOnBtnL, this);
			m_pW->setCbBtn("R", sOnBtnR, this);
			m_pW->setCbBtn("U", sOnBtnU, this);
			m_pW->setCbBtn("D", sOnBtnD, this);
			m_pW->setCbBtn("F", sOnBtnF, this);
			m_pW->setCbBtn("B", sOnBtnB, this);
			m_pW->setCbBtn("Sl", sOnBtnSl, this);
			m_pW->setCbBtn("Ss", sOnBtnSs, this);
			m_pW->setBtnVisible("L", true);
			m_pW->setBtnVisible("R", true);
			m_pW->setBtnVisible("U", true);
			m_pW->setBtnVisible("D", true);
			m_pW->setBtnVisible("F", true);
			m_pW->setBtnVisible("B", true);
			m_pW->setBtnVisible("Sl", true);
			m_pW->setBtnVisible("Ss", true);

			clear();
		}
	}

	bool _ARmeasureCalibDofs::saveCalib(void)
	{
		picojson::object o;
		o.insert(make_pair("name", "calib"));

		vFloat3 vDofsP = m_pA->getDofsP();
		picojson::array v;
		v.push_back(value(vDofsP.x));
		v.push_back(value(vDofsP.y));
		v.push_back(value(vDofsP.z));
		o.insert(make_pair("vDofsP", value(v)));
		v.clear();

		string f = picojson::value(o).serialize();

		_File *pF = new _File();
		IF_F(!pF->open(&m_fKiss, ios::out));
		pF->write((uint8_t *)f.c_str(), f.length());
		pF->close();
		DEL(pF);

		m_drawMsg = "Calibration saved";
		sleep(1);

		return true;
	}

	// callbacks
	void _ARmeasureCalibDofs::sOnBtnAction(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->action();
	}

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

	void _ARmeasureCalibDofs::sOnBtnL(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->L();
	}

	void _ARmeasureCalibDofs::sOnBtnR(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->R();
	}

	void _ARmeasureCalibDofs::sOnBtnU(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->U();
	}

	void _ARmeasureCalibDofs::sOnBtnD(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->D();
	}

	void _ARmeasureCalibDofs::sOnBtnF(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->F();
	}

	void _ARmeasureCalibDofs::sOnBtnB(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->B();
	}

	void _ARmeasureCalibDofs::sOnBtnSl(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Sl();
	}

	void _ARmeasureCalibDofs::sOnBtnSs(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalibDofs *)pInst)->Ss();
	}

	// UI handler
	void _ARmeasureCalibDofs::action(void)
	{
		IF_(check() < 0);

		Mat mV;
		m_pV->BGR()->m()->copyTo(mV);

	}

	void _ARmeasureCalibDofs::clear(void)
	{
	}

	void _ARmeasureCalibDofs::save(void)
	{
	}

	void _ARmeasureCalibDofs::mode(uint32_t f)
	{
		NULL_(m_pSC);

		if (f & 1) //long push
			m_pSC->transit("V");
		else
			m_pSC->transit("calibCam");
	}

	void _ARmeasureCalibDofs::L(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.x += m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::R(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.x -= m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::U(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.x += m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::D(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.x -= m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::F(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.x += m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::B(void)
	{
		NULL_(m_pA);
		vFloat3 v = m_pA->getDofsP();
		v.x -= m_step;
		m_pA->setDofsP(v);
	}

	void _ARmeasureCalibDofs::Sl(void)
	{
		m_step *= 10.0;
	}

	void _ARmeasureCalibDofs::Ss(void)
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
						   15,
						   Scalar(0, 255, 0),
						   -1,
						   cv::LINE_AA,
						   false);
		}
	}

	void _ARmeasureCalibDofs::drawMsg(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);
		IF_(m_drawMsg.empty());

		m_pFt->putText(*pM, m_drawMsg,
					   Point(20, 20),
					   40,
					   Scalar(255, 0, 0),
					   -1,
					   cv::LINE_AA,
					   false);
	}

	void _ARmeasureCalibDofs::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		IF_(!bActive());

		this->_StateBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getFrame();
		NULL_(pF);
		Mat *pMw = pF->m();
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
		drawMsg(pMw);
	}

}
