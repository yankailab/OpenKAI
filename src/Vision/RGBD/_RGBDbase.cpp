/*
 * _RGBDbase.cpp
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#include "_RGBDbase.h"

namespace kai
{

	_RGBDbase::_RGBDbase()
	{
		m_pTPP = nullptr;

		m_devFPSd = 30;
		m_vSizeD.set(1280, 720);
		m_vRangeD.set(0, FLT_MAX);

		m_bDepth = true;
		m_bIR = false;
		m_btRGB = false;
		m_btDepth = false;
		m_bConfidence = true;
		m_fConfidenceThreshold = 0.0;

#ifdef USE_OPENCV
		m_dScale = 1.0;
		m_dOfs = 0.0;
		m_nHistLev = 128;
		m_iHistFrom = 0;
		m_minHistD = 0.25;
		m_bDebugDepth = 0;
#endif
	}

	_RGBDbase::~_RGBDbase()
	{
		DEL(m_pTPP);
	}

	int _RGBDbase::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("devFPSd", &m_devFPSd);
		pK->v("vSizeD", &m_vSizeD);
		pK->v("vRangeD", &m_vRangeD);

		pK->v("bDepth", &m_bDepth);
		pK->v("bIR", &m_bIR);
		pK->v("btRGB", &m_btRGB);
		pK->v("btDepth", &m_btDepth);
		pK->v("bConfidence", &m_bConfidence);
		pK->v("fConfidenceThreshold", &m_fConfidenceThreshold);

#ifdef USE_OPENCV
		pK->v("dScale", &m_dScale);
		pK->v("dOfs", &m_dOfs);
		pK->v("nHistLev", &m_nHistLev);
		pK->v("iHistFrom", &m_nHistLev);
		pK->v("minHistD", &m_minHistD);
		pK->v("bDebugDepth", &m_bDebugDepth);
#endif

		return OK_OK;
	}

	int _RGBDbase::link(void)
	{
		CHECK_(this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		return OK_OK;
	}

	bool _RGBDbase::open(void)
	{
		return false;
	}

	void _RGBDbase::close(void)
	{
	}

	int _RGBDbase::check(void)
	{
		return _VisionBase::check();
	}

	void _RGBDbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_VisionBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		//		pC->addMsg("", 0);
	}

#ifdef USE_OPENCV
	Frame *_RGBDbase::getFrameD(void)
	{
		return &m_fDepth;
	}

	vFloat2 _RGBDbase::getRangeD(void)
	{
		return m_vRangeD;
	}

	float _RGBDbase::d(const vFloat4 &bb)
	{
		IF__(m_fDepth.bEmpty(), -1.0);

		Size s = m_fDepth.size();
		vInt4 vBB;
		vBB.x = bb.x * s.width;
		vBB.y = bb.y * s.height;
		vBB.z = bb.z * s.width;
		vBB.w = bb.w * s.height;

		if (vBB.x < 0)
			vBB.x = 0;
		if (vBB.y < 0)
			vBB.y = 0;
		if (vBB.z > s.width)
			vBB.z = s.width;
		if (vBB.w > s.height)
			vBB.w = s.height;

		return d(vBB);
	}

	float _RGBDbase::d(const vInt4 &bb)
	{
		IF__(m_fDepth.bEmpty(), -1.0);

		vector<int> vHistLev = {m_nHistLev};
		vector<float> vRange = {m_vRangeD.x, m_vRangeD.y};
		vector<int> vChannel = {0};

		Rect r = bb2Rect(bb);
		Mat mRoi = (*m_fDepth.m())(r);
		vector<Mat> vRoi = {mRoi};
		Mat mHist;
		cv::calcHist(vRoi, vChannel, Mat(),
					 mHist, vHistLev, vRange,
					 false // accumulate
		);

		int nMinHist = m_minHistD * mRoi.cols * mRoi.rows;
		int nPix = 0;
		int i;
		for (i = m_iHistFrom; i < m_nHistLev; i++)
		{
			nPix += (int)mHist.at<float>(i);
			if (nPix >= nMinHist)
				break;
		}

		return (m_vRangeD.x + (((float)i) / (float)m_nHistLev) * m_vRangeD.len());
	}

	void _RGBDbase::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_VisionBase::draw(pFrame);
		IF_(check() != OK_OK);
		IF_(m_fRGB.bEmpty());

		if (m_bDebugDepth)
		{
			Frame *pF = (Frame *)pFrame;
//			pF->copy(m_fDepth);

			Mat *pM = pF->m();
			IF_(pM->empty());

			vFloat4 vRoi(0.4, 0.4, 0.6, 0.6);

			vFloat4 bb;
			bb.x = vRoi.x * pM->cols;
			bb.y = vRoi.y * pM->rows;
			bb.z = vRoi.z * pM->cols;
			bb.w = vRoi.w * pM->rows;
			Rect r = bb2Rect(bb);
			rectangle(*pM, r, Scalar(128, 128, 128), 2);

			putText(*pM, f2str(d(vRoi)),
					Point(r.x + 15, r.y + 25),
					FONT_HERSHEY_SIMPLEX, 0.6, Scalar(128, 128, 128), 2);
		}
	}
#endif

#ifdef WITH_3D
	int _RGBDbase::getPointCloud(_PCframe* pPCframe, int nPmax)
	{

	}
#endif


}
