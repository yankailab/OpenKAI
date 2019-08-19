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
	m_vRange.x = 0.8;
	m_vRange.y = 16.0;
	m_minHistD = 0.25;
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

	pK->v("rFrom", &m_vRange.x);
	pK->v("rTo", &m_vRange.y);

	string iName = "";
	F_INFO(pK->v("depthWindow", &iName));
	m_pDepthWin = (Window*) (pK->root()->getChildInst(iName));

	return true;
}

double _DepthVisionBase::d(vDouble4* pROI)
{
	IF__(!pROI, -1.0);
	IF__(m_fDepth.bEmpty(),-1.0);

	Size s = m_fDepth.size();

	vInt4 iR;
	iR.x = pROI->x * s.width;
	iR.y = pROI->y * s.height;
	iR.z = pROI->z * s.width;
	iR.w = pROI->w * s.height;

	if (iR.x < 0)
		iR.x = 0;
	if (iR.y < 0)
		iR.y = 0;
	if (iR.z > s.width)
		iR.z = s.width;
	if (iR.w > s.height)
		iR.w = s.height;

	return d(&iR);
}

double _DepthVisionBase::d(vInt4* pROI)
{
	IF__(!pROI, -1.0);
	IF__(m_fDepth.bEmpty(),-1.0);

    vector<int> vHistLev = { m_nHistLev };
	vector<float> vRange = { (float)m_vRange.x, (float)m_vRange.y };
	vector<int> vChannel = { 0 };

	Rect r = convertBB(*pROI);
	Mat mRoi = (*m_fDepth.m())(r);
	vector<Mat> vRoi = {mRoi};
	Mat mHist;
	cv::calcHist(vRoi, vChannel, Mat(),
	            mHist, vHistLev, vRange,
	            true	//accumulate
				);

	int nMinHist = m_minHistD * mRoi.cols * mRoi.rows;
	int i;
	for(i=0; i<m_nHistLev; i++)
	{
		if(mHist.at<float>(i) >= (float)nMinHist)break;
	}

	return m_vRange.x + (((double)i)/(double)m_nHistLev) * (m_vRange.y - m_vRange.x);
}

Frame* _DepthVisionBase::Depth(void)
{
	return &m_fDepth;
}

bool _DepthVisionBase::draw(void)
{
	IF_F(!this->_VisionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Frame* pFrame = pWin->getFrame();

	IF_F(m_depthShow.bEmpty());
	IF_F(!m_pDepthWin);

	m_pDepthWin->getFrame()->copy(*m_depthShow.m());

	return true;
}

}
