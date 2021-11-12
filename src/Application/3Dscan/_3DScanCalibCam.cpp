/*
 * _3DScanCalibCam.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_3DScanCalibCam.h"

namespace kai
{

	_3DScanCalibCam::_3DScanCalibCam()
	{
		m_pR = NULL;
		m_pW = NULL;

		m_iPreview = 0;
		m_tPreview = 3;
		m_vChessBoardSize.init(9, 6);
		m_squareSize = 1.0;
		m_fCalib = "";

		m_drawCol = Scalar(0, 255, 0);
		m_pFt = NULL;
	}

	_3DScanCalibCam::~_3DScanCalibCam()
	{
	}

	bool _3DScanCalibCam::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("fCalib", &m_fCalib);
		pK->v("tPreview", &m_tPreview);
		pK->v("vChessBoardSize", &m_vChessBoardSize);
		pK->v("squareSize", &m_squareSize);

		for (int i = 0; i < m_vChessBoardSize.x; i++)
		{
			for (int j = 0; j < m_vChessBoardSize.y; j++)
				m_vPo.push_back(cv::Point3f(j * m_squareSize, i * m_squareSize, 0));
		}

		string n;

		n = "";
		pK->v("_Remap", &n);
		m_pR = (_Remap *)(pK->getInst(n));
		IF_Fl(!m_pR, n + " not found");

		n = "";
		pK->v("_WindowCV", &n);
		m_pW = (_WindowCV *)(pK->getInst(n));
		IF_Fl(!m_pW, n + " not found");

		return true;
	}

	bool _3DScanCalibCam::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _3DScanCalibCam::check(void)
	{
		NULL__(m_pW, -1);
		NULL__(m_pR, -1);
		IF__(m_pR->BGR()->bEmpty(), -1);

		return 0;
	}

	void _3DScanCalibCam::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			this->_StateBase::update();
			updateCalib();

			m_pT->autoFPSto();
		}
	}

	void _3DScanCalibCam::updateCalib(void)
	{
		IF_(!bActive());
		if (bStateChanged())
		{
			m_pW->setBtnVisibleAll(false);
			m_pW->setCbBtn("Action", sOnBtnAction, this);
			m_pW->setCbBtn("Save", sOnBtnSave, this);
			m_pW->setCbBtn("Clear", sOnBtnClear, this);
			m_pW->setCbBtn("Mode", sOnBtnMode, this);
			m_pW->setBtnLabel("Action", "A");
			m_pW->setBtnLabel("Mode", "CC");

			m_pW->setBtnVisible("Action", true);
			m_pW->setBtnVisible("Save", true);
			m_pW->setBtnVisible("Clear", true);
			m_pW->setBtnVisible("Mode", true);
			clear();
		}
	}

	bool _3DScanCalibCam::camCalib(void)
	{
		IF_F(m_vvPpo.empty());

		Mat mR, mT;
		cv::Size s = m_pR->BGR()->size();
		calibrateCamera(m_vvPpo, m_vvPimg, s, m_mC, m_mD, mR, mT);
		m_mC.at<double>(0, 0) /= (double)s.width;  //Fx
		m_mC.at<double>(1, 1) /= (double)s.height; //Fy
		m_mC.at<double>(0, 2) /= (double)s.width;  //Cx
		m_mC.at<double>(1, 2) /= (double)s.height; //Cy
		m_pR->setCamMatrices(m_mC, m_mD);

		clear();
		return true;
	}

	bool _3DScanCalibCam::saveCalib(void)
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

		string f = picojson::value(o).serialize();

		_File *pF = new _File();
		IF_F(!pF->open(&m_fCalib, ios::out));
		pF->write((uint8_t *)f.c_str(), f.length());
		pF->close();
		DEL(pF);

		return true;
	}

	// callbacks
	void _3DScanCalibCam::sOnBtnAction(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibCam *)pInst)->action();
	}

	void _3DScanCalibCam::sOnBtnClear(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibCam *)pInst)->clear();
	}

	void _3DScanCalibCam::sOnBtnMode(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibCam *)pInst)->mode(f);
	}

	void _3DScanCalibCam::sOnBtnSave(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_3DScanCalibCam *)pInst)->save(f);
	}

	// UI handler
	void _3DScanCalibCam::action(void)
	{
		IF_(check() < 0);

		m_pR->BGR()->m()->copyTo(m_mCalib);

		Mat mGray;
		vector<Point2f> vPcorner; // vector to store the pixel coordinates of detected checker board corners
		cvtColor(m_mCalib, mGray, cv::COLOR_BGR2GRAY);

		// If desired number of corners are found in the image then bSuccess = true
		IF_(!cv::findChessboardCorners(mGray,
									   cv::Size(m_vChessBoardSize.y, m_vChessBoardSize.x),
									   vPcorner,
									   cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE));

		TermCriteria criteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 30, 0.001);

		// refining pixel coordinates for given 2d points.
		cornerSubPix(mGray, vPcorner, cv::Size(11, 11), cv::Size(-1, -1), criteria);

		// Displaying the detected corner points on the checker board
		drawChessboardCorners(m_mCalib, cv::Size(m_vChessBoardSize.y, m_vChessBoardSize.x), vPcorner, true);
		m_iPreview = 1;

		m_vvPpo.push_back(m_vPo);
		m_vvPimg.push_back(vPcorner);
	}

	void _3DScanCalibCam::clear(void)
	{
		m_vvPpo.clear();
		m_vvPimg.clear();
	}

	void _3DScanCalibCam::save(uint32_t f)
	{
		if (f & 1) //long push
		{
			saveCalib();
		}
		else
		{
			camCalib();
		}
	}

	void _3DScanCalibCam::mode(uint32_t f)
	{
		NULL_(m_pSC);

		if (f & 1) //long push
			m_pSC->transit("vertex");
		else
			m_pSC->transit("calibDofs");
	}

	// UI draw
	void _3DScanCalibCam::drawCalibData(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		int x = 100;
		int y = 50;
		int s = 20;
		int n = 5;
		int nd = 6;

		string c[5];
		c[0] = "Nsample: " + i2str(m_vvPimg.size());
		if (m_mC.empty())
		{
			n = 1;
		}
		else
		{
			c[1] = "Fx: " + lf2str(m_mC.at<double>(0, 0), nd);
			c[2] = "Fy: " + lf2str(m_mC.at<double>(1, 1), nd);
			c[3] = "Cx: " + lf2str(m_mC.at<double>(0, 2), nd);
			c[4] = "Cy: " + lf2str(m_mC.at<double>(1, 2), nd);
		}

		for (int i = 0; i < n; i++)
		{
			m_pFt->putText(*pM, c[i],
						   Point(x, y + s * i),
						   20,
						   Scalar(0, 255, 0),
						   -1,
						   cv::LINE_AA,
						   false);
		}
	}

	void _3DScanCalibCam::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		IF_(!bActive());

		this->_StateBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Mat *pMw = pWin->getNextFrame()->m();
		IF_(pMw->empty());
		m_pFt = pWin->getFont();

		if(m_iPreview == 2)
		{
			sleep(m_tPreview);
			m_iPreview = 0;
		}

		Mat mV;
		if (m_iPreview == 1)
		{
			m_mCalib.copyTo(mV);
			m_iPreview++;			
		}
		else
		{
			m_pR->BGR()->m()->copyTo(mV);
		}

		Rect r;
		r.x = 0;
		r.y = 0;
		r.width = mV.cols;
		r.height = mV.rows;
		mV.copyTo((*pMw)(r));

		drawCalibData(pMw);
	}

}
