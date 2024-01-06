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
        m_pTPP = NULL;

		m_deviceURI = "";
        m_bOpen = false;
        m_vSizeRGB.set(1600, 1200);
		m_vSizeD.set(1280, 720);
		m_vRz.set(0.0, FLT_MAX);
		m_tFrameInterval = 0;

        m_bRGB = true;
        m_bDepth = true;
        m_bIR = false;
        m_btRGB = false;
        m_btDepth = false;
        m_fConfidenceThreshold = 0.0;

        m_psmRGB = NULL;
        m_psmDepth = NULL;
        m_psmTransformedRGB = NULL;
        m_psmTransformedDepth = NULL;
        m_psmIR = NULL;

#ifdef USE_OPENCV
		m_vRange.set(0.8,16.0);
		m_dScale = 1.0;
		m_dOfs = 0.0;
		m_nHistLev = 128;
		m_iHistFrom = 0;
		m_minHistD = 0.25;
#endif
    }

    _RGBDbase::~_RGBDbase()
    {
        DEL(m_pTPP);
    }

    bool _RGBDbase::init(void *pKiss)
    {
        IF_F(!_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

		pK->v("URI", &m_deviceURI);
        pK->v("vSizeRGB", &m_vSizeRGB);
        pK->v("vSizeD", &m_vSizeD);
		pK->v("vRz", &m_vRz);
		pK->v("tFrameInterval", &m_tFrameInterval);

        pK->v("bRGB", &m_bRGB);
        pK->v("bDepth", &m_bDepth);
        pK->v("bIR", &m_bIR);
        pK->v("btRGB", &m_btRGB);
        pK->v("btDepth", &m_btDepth);
        pK->v("fConfidenceThreshold", &m_fConfidenceThreshold);

#ifdef USE_OPENCV
		pK->v("vRange", &m_vRange);
		pK->v("dScale", &m_dScale);
		pK->v("dOfs", &m_dOfs);
		pK->v("nHistLev", &m_nHistLev);
		pK->v("iHistFrom", &m_nHistLev);
		pK->v("minHistD", &m_minHistD);
#endif

        return true;
    }

    bool _RGBDbase::link(void)
    {
        IF_F(!this->_ModuleBase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

        n = "";
        pK->v("_SHMrgb", &n);
        m_psmRGB = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMtransformedRGB", &n);
        m_psmTransformedRGB = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMdepth", &n);
        m_psmDepth = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMtransformedDepth", &n);
        m_psmTransformedDepth = (_SharedMem *)(pK->getInst(n));

        n = "";
        pK->v("_SHMir", &n);
        m_psmIR = (_SharedMem *)(pK->getInst(n));

        return true;
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
        NULL__(m_pT, -1);

        if (m_bRGB)
        {
            NULL__(m_psmRGB, -1);
            IF__(!m_psmRGB->open(), -1);
        }

        if (m_bDepth)
        {
            NULL__(m_psmDepth, -1);
            IF__(!m_psmDepth->open(), -1);
        }

        if (m_btRGB)
        {
            NULL__(m_psmTransformedRGB, -1);
            IF__(!m_psmTransformedRGB->open(), -1);
        }

        if (m_btDepth)
        {
            NULL__(m_psmTransformedDepth, -1);
            IF__(!m_psmTransformedDepth->open(), -1);
        }

        if (m_bIR)
        {
            NULL__(m_psmIR, -1);
            IF__(!m_psmIR->open(), -1);
        }

        return _ModuleBase::check();
    }

	void _RGBDbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
//		pC->addMsg("nState: " + i2str(m_vStates.size()), 0);
	}

#ifdef USE_OPENCV
	Frame *_RGBDbase::RGB(void)
	{
		return &m_fRGB;
	}

	Frame *_RGBDbase::Depth(void)
	{
		return &m_fDepth;
	}

	float _RGBDbase::d(const vFloat4& bb)
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

	float _RGBDbase::d(const vInt4& bb)
	{
		IF__(m_fDepth.bEmpty(), -1.0);

		vector<int> vHistLev = {m_nHistLev};
		vector<float> vRange = {m_vRange.x, m_vRange.y};
		vector<int> vChannel = {0};

		Rect r = bb2Rect(bb);
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

		return (m_vRange.x + (((float)i) / (float)m_nHistLev) * m_vRange.len());
	}

    void _RGBDbase::draw(void* pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);
		IF_(m_fRGB.bEmpty());

		Frame *pF = (Frame*)pFrame;

		pF->copy(m_fRGB);

			// Mat *pM = pF->m();
			// IF_(pM->empty());
			// Rect r = bb2Rect(bbScale(m_bbDraw, pM->cols, pM->rows));
			// Mat m;
			// cv::resize(*m_fBGR.m(), m, Size(r.width, r.height));

			// m.copyTo((*pM)(r));
	}
#endif

}
