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
	m_wD = 1280;
	m_hD = 720;

	m_nHistLev = 128;
	m_iHistFrom = 0;
	m_dScale = 1.0;
	m_vRange.x = 0.8;
	m_vRange.y = 16.0;
	m_minHistD = 0.25;
	m_vKpos.x = 0.0;
	m_vKpos.y = 0.0;
	m_vKpos.z = 1.0;
	m_vKpos.w = 1.0;
	m_vDRoi.init();
}

_DepthVisionBase::~_DepthVisionBase()
{
}

bool _DepthVisionBase::init(void* pKiss)
{
	IF_F(!this->_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("wD",&m_wD);
	pK->v("hD",&m_hD);
	pK->v("nHistLev",&m_nHistLev);
	pK->v("minHistD",&m_minHistD);
	pK->v("vRange", &m_vRange);
	pK->v("vKpos", &m_vKpos);

	string iName = "";
	F_INFO(pK->v("depthWindow", &iName));
	m_pDepthWin = (Window*) (pK->root()->getChildInst(iName));

	return true;
}

float _DepthVisionBase::d(vFloat4* pROI)
{
	IF__(!pROI, -1.0);
	IF__(m_fDepth.bEmpty(), -1.0);

	Size s = m_fDepth.size();

	m_vDRoi.x = (m_vKpos.x + (pROI->x * m_vKpos.z)) * s.width;
	m_vDRoi.y = (m_vKpos.y + (pROI->y * m_vKpos.w)) * s.height;
	m_vDRoi.z = (m_vKpos.x + (pROI->z * m_vKpos.z)) * s.width;
	m_vDRoi.w = (m_vKpos.y + (pROI->w * m_vKpos.w)) * s.height;

	if (m_vDRoi.x < 0)
		m_vDRoi.x = 0;
	if (m_vDRoi.y < 0)
		m_vDRoi.y = 0;
	if (m_vDRoi.z > s.width)
		m_vDRoi.z = s.width;
	if (m_vDRoi.w > s.height)
		m_vDRoi.w = s.height;

	return d(&m_vDRoi);
}

float _DepthVisionBase::d(vInt4* pROI)
{
	IF__(!pROI, -1.0);
	IF__(m_fDepth.bEmpty(), -1.0);

    vector<int> vHistLev = { m_nHistLev };
	vector<float> vRange = { m_vRange.x, m_vRange.y };
	vector<int> vChannel = { 0 };

	Rect r = convertBB(*pROI);
	Mat mRoi = (*m_fDepth.m())(r);
	vector<Mat> vRoi = {mRoi};
	Mat mHist;
	cv::calcHist(vRoi, vChannel, Mat(),
	            mHist, vHistLev, vRange,
	            false	//accumulate
				);

	int nMinHist = m_minHistD * mRoi.cols * mRoi.rows;
	int nPix = 0;
	int i;
	for(i=m_iHistFrom; i<m_nHistLev; i++)
	{
		nPix += (int)mHist.at<float>(i);
		if(nPix >= nMinHist)break;
	}

	return m_dScale * (m_vRange.x + (((float)i)/(float)m_nHistLev) * m_vRange.len());
}

Frame* _DepthVisionBase::Depth(void)
{
	return &m_fDepth;
}

void _DepthVisionBase::draw(void)
{
	this->_VisionBase::draw();

	if(checkWindow())
	{
		Window* pWin = (Window*)this->m_pWindow;
		Frame* pFrame = pWin->getFrame();
		Mat* pMat = pFrame->m();

		if(m_bDebug)
		{
			vInt2 cs;
			cs.x = pMat->cols;
			cs.y = pMat->rows;

			vFloat4 bb;
			bb.x = 0.4;
			bb.y = 0.4;
			bb.z = 0.6;
			bb.w = 0.6;
			Rect r = convertBB<vInt4>(convertBB(bb, cs));

			rectangle(*pMat, r, Scalar(0,255,0), 1);

//			putText(*pMat, f2str(d(&bb)),
//						Point(r.x + 15, r.y + 25),
//						FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0,255,0), 1);

			r = convertBB<vInt4>(m_vDRoi);
			rectangle(*pMat, r, Scalar(0,255,255), 2);
		}
	}

	IF_(m_depthShow.bEmpty());
	IF_(!m_pDepthWin);
	m_pDepthWin->getFrame()->copy(*m_depthShow.m());
}

}
