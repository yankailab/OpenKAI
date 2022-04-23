/*
 * _DepthVisionBase.cpp
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#include "_DepthVisionBase.h"

namespace kai
{

	_DepthVisionBase::_DepthVisionBase()
	{
		m_pDepthWin = NULL;
		m_vDsize.set(1280, 720);

		m_nHistLev = 128;
		m_iHistFrom = 0;
		m_dScale = 1.0;
		m_vRange.x = 0.8;
		m_vRange.y = 16.0;
		m_minHistD = 0.25;
		m_vDroi.init();
	}

	_DepthVisionBase::~_DepthVisionBase()
	{
	}

	bool _DepthVisionBase::init(void *pKiss)
	{
		IF_F(!this->_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vDsize", &m_vDsize);
		pK->v("nHistLev", &m_nHistLev);
		pK->v("minHistD", &m_minHistD);
		pK->v("vRange", &m_vRange);

		string n = "";
		F_INFO(pK->v("depthWindow", &n));
		m_pDepthWin = (_WindowCV *)(pK->getInst(n));

		return true;
	}

	float _DepthVisionBase::d(vFloat4 *pROI)
	{
		IF__(!pROI, -1.0);
		IF__(m_fDepth.bEmpty(), -1.0);

		Size s = m_fDepth.size();
		m_vDroi.x = pROI->x * s.width;
		m_vDroi.y = pROI->y * s.height;
		m_vDroi.z = pROI->z * s.width;
		m_vDroi.w = pROI->w * s.height;

		if (m_vDroi.x < 0)
			m_vDroi.x = 0;
		if (m_vDroi.y < 0)
			m_vDroi.y = 0;
		if (m_vDroi.z > s.width)
			m_vDroi.z = s.width;
		if (m_vDroi.w > s.height)
			m_vDroi.w = s.height;

		return d(&m_vDroi);
	}

	float _DepthVisionBase::d(vInt4 *pROI)
	{
		IF__(!pROI, -1.0);
		IF__(m_fDepth.bEmpty(), -1.0);

		vector<int> vHistLev = {m_nHistLev};
		vector<float> vRange = {m_vRange.x, m_vRange.y};
		vector<int> vChannel = {0};

		Rect r = bb2Rect(*pROI);
		Mat mRoi = (*m_fDepth.m())(r);
		vector<Mat> vRoi = {mRoi};
		Mat mHist;
		cv::calcHist(vRoi, vChannel, Mat(),
					 mHist, vHistLev, vRange,
					 false //accumulate
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

		return /*m_dScale **/ (m_vRange.x + (((float)i) / (float)m_nHistLev) * m_vRange.len());
	}

	Frame *_DepthVisionBase::Depth(void)
	{
		return &m_fDepth;
	}

	void _DepthVisionBase::cvDraw(void *pWindow)
	{
#ifdef WITH_UI
#ifdef USE_OPENCV
		NULL_(pWindow);
		this->_VisionBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getNextFrame();
		NULL_(pF);
		Mat *pM = pF->m();
		IF_(pM->empty());

		if (m_bDebug)
		{
			vInt2 cs;
			cs.x = pM->cols;
			cs.y = pM->rows;

			vFloat4 vRoi(0.4, 0.4, 0.6, 0.6);

			vFloat4 bb;
			bb.x = vRoi.x * pM->cols;
			bb.y = vRoi.y * pM->rows;
			bb.z = vRoi.z * pM->cols;
			bb.w = vRoi.w * pM->rows;
			Rect r = bb2Rect(bb);
			rectangle(*pM, r, Scalar(0, 255, 0), 1);

			putText(*pM, f2str(d(&vRoi)),
					Point(r.x + 15, r.y + 25),
					FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 1);
		}

		IF_(m_fDepthShow.bEmpty());
		IF_(!m_pDepthWin);
		m_pDepthWin->getNextFrame()->copy(*m_fDepthShow.m());
#endif
#endif
	}

}
