/*
 * _ARmeasureCalib.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_ARmeasureCalib.h"

namespace kai
{

	_ARmeasureCalib::_ARmeasureCalib()
	{
		m_pA = NULL;
		m_pV = NULL;
		m_pW = NULL;

		m_vChessBoardSize.init(9, 6);
		m_squareSize = 1.0;
		m_fKiss = "";

		m_drawCol = Scalar(0, 255, 0);
		m_pFt = NULL;
	}

	_ARmeasureCalib::~_ARmeasureCalib()
	{
	}

	bool _ARmeasureCalib::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vChessBoardSize", &m_vChessBoardSize);
		pK->v("squareSize", &m_squareSize);

		for (int i = 0; i < m_vChessBoardSize.x; i++)
		{
			for (int j = 0; j < m_vChessBoardSize.y; j++)
				m_vPo.push_back(cv::Point3f(j * m_squareSize, i * m_squareSize, 0));
		}

		pK->v("fKiss", &m_fKiss);

		string n;

		n = "";
		pK->v("_ARmeasure", &n);
		m_pA = (_ARmeasure *)(pK->getInst(n));
		IF_Fl(!m_pA, n + " not found");

		n = "";
		pK->v("_Remap", &n);
		m_pV = (_Remap *)(pK->getInst(n));
		IF_Fl(!m_pV, n + " not found");

		n = "";
		pK->v("_WindowCV", &n);
		m_pW = (_WindowCV *)(pK->getInst(n));
		IF_Fl(!m_pW, n + " not found");

		return true;
	}

	bool _ARmeasureCalib::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ARmeasureCalib::check(void)
	{
		NULL__(m_pA, -1);
		NULL__(m_pW, -1);
		NULL__(m_pV, -1);
		IF__(m_pV->BGR()->bEmpty(), -1);

		return 0;
	}

	void _ARmeasureCalib::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			this->_StateBase::update();
			updateCalib();

			m_pT->autoFPSto();
		}
	}

	void _ARmeasureCalib::updateCalib(void)
	{
		IF_(!bActive());
		if (bStateChanged())
		{
			m_pW->setCbBtn("Action", sOnBtnAction, this);
			m_pW->setCbBtn("Clear", sOnBtnClear, this);
			m_pW->setCbBtn("Mode", sOnBtnMode, this);

			m_pW->setBtnLabel("Action", "Add");
			m_pW->setBtnLabel("Mode", "V");

			clear();
		}
	}

	bool _ARmeasureCalib::camCalib(void)
	{
		Mat mGray;
		vector<Point2f> vPcorner; // vector to store the pixel coordinates of detected checker board corners
		bool bSuccess;

		for (int i{0}; i < m_vImg.size(); i++)
		{
			cvtColor(m_vImg[i], mGray, cv::COLOR_BGR2GRAY);

			// If desired number of corners are found in the image then bSuccess = true
			bSuccess = cv::findChessboardCorners(mGray,
												 cv::Size(m_vChessBoardSize.y, m_vChessBoardSize.x),
												 vPcorner,
												 cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);
			IF_CONT(!bSuccess);

			TermCriteria criteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 30, 0.001);

			// refining pixel coordinates for given 2d points.
			cornerSubPix(mGray, vPcorner, cv::Size(11, 11), cv::Size(-1, -1), criteria);

			// Displaying the detected corner points on the checker board
			//                drawChessboardCorners(m, cv::Size(m_vChessBoardSize.y, m_vChessBoardSize.x), vPcorner, bSuccess);

			m_vvPpo.push_back(m_vPo);
			m_vvPimg.push_back(vPcorner);
		}

		IF_F(m_vvPpo.empty());

		Mat mR, mT;
		calibrateCamera(m_vvPpo, m_vvPimg, cv::Size(mGray.rows, mGray.cols), m_mC, m_mD, mR, mT);
		m_mC.at<double>(0, 0) /= (double)mGray.cols; //Fx
		m_mC.at<double>(1, 1) /= (double)mGray.rows; //Fy
		m_mC.at<double>(0, 2) /= (double)mGray.cols; //Cx
		m_mC.at<double>(1, 2) /= (double)mGray.rows; //Cy

		return true;
	}

	bool _ARmeasureCalib::saveCalib(void)
	{
		double Fx = m_mC.at<double>(0, 0);
		double Fy = m_mC.at<double>(1, 1);
		double Cx = m_mC.at<double>(0, 2);
		double Cy = m_mC.at<double>(1, 2);

		double k1 = m_mD.at<double>(0, 0);
		double k2 = m_mD.at<double>(0, 1);
		double p1 = m_mD.at<double>(0, 2);
		double p2 = m_mD.at<double>(0, 3);
		double k3 = m_mD.at<double>(0, 4);

		picojson::object o;
		o.insert(make_pair("name", "calib"));
		o.insert(make_pair("Fx", value(Fx)));
		o.insert(make_pair("Fy", value(Fy)));
		o.insert(make_pair("Cx", value(Cx)));
		o.insert(make_pair("Cy", value(Cy)));

		o.insert(make_pair("k1", value(k1)));
		o.insert(make_pair("k2", value(k2)));
		o.insert(make_pair("p1", value(p1)));
		o.insert(make_pair("p2", value(p2)));
		o.insert(make_pair("k3", value(k3)));

		// picojson::array v;
		// v.push_back(value(pP->m_coTx));
		// v.push_back(value(pP->m_coTy));
		// v.push_back(value(pP->m_coTz));
		// o.insert(make_pair("vOffsetCt", value(v)));
		// v.clear();
		// v.push_back(value(pP->m_coRx));
		// v.push_back(value(pP->m_coRy));
		// v.push_back(value(pP->m_coRz));
		// o.insert(make_pair("vOffsetCr", value(v)));
		// v.clear();

		string f = picojson::value(o).serialize();

		_File *pF = new _File();
		IF_F(!pF->open(&m_fKiss, ios::out));
		pF->write((uint8_t *)f.c_str(), f.length());
		pF->close();
		DEL(pF);

		return true;
	}

	// callbacks
	void _ARmeasureCalib::sOnBtnAction(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalib *)pInst)->action();
	}

	void _ARmeasureCalib::sOnBtnClear(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalib *)pInst)->clear();
	}

	void _ARmeasureCalib::sOnBtnMode(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasureCalib *)pInst)->mode(f);
	}

	// UI handler
	void _ARmeasureCalib::action(void)
	{
		IF_(check() < 0);

		Mat mV;
		m_pV->BGR()->m()->copyTo(mV);

		m_vImg.push_back(mV);
	}

	void _ARmeasureCalib::clear(void)
	{
		m_vImg.clear();
		m_vvPpo.clear();
		m_vvPimg.clear();
	}

	void _ARmeasureCalib::mode(uint32_t f)
	{
		NULL_(m_pSC);

		if (f & 1)
		{
			//long pushed, save calib data
		}

		m_pSC->transit("V");
	}

	// UI draw
	void _ARmeasureCalib::cvDraw(void *pWindow)
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
	}

}
