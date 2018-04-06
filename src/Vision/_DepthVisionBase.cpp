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
	m_pDepth = NULL;
	m_pDepthWin = NULL;
	m_range.x = 0.0;
	m_range.y = 10.0;
	m_wD = 1280;
	m_hD = 720;
}

_DepthVisionBase::~_DepthVisionBase()
{
	reset();
}

bool _DepthVisionBase::init(void* pKiss)
{
	IF_F(!this->_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,wD);
	KISSm(pK,hD);
	F_INFO(pK->v("rFrom", &m_range.x));
	F_INFO(pK->v("rTo", &m_range.y));

	m_pDepth = new Frame();

	return true;
}

bool _DepthVisionBase::link(void)
{
	IF_F(!this->_VisionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("depthWindow", &iName));
	m_pDepthWin = (Window*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void _DepthVisionBase::reset(void)
{
	this->_VisionBase::reset();

	DEL(m_pDepth);
}

Frame* _DepthVisionBase::depth(void)
{
	return m_pDepth;
}

vDouble2 _DepthVisionBase::range(void)
{
	return m_range;
}

double _DepthVisionBase::dist(Rect* pR)
{
	NULL_F(pR);

	GpuMat gMat;
	GpuMat gMat2;
	GpuMat gHist;
	Mat cHist;

//	NULL_F(m_pDepth);
//	IF_F(m_pDepth->empty());
//	gMat = *(m_pDepth->getGMat());
//	gMat2 = GpuMat(gMat, *pR);

	int intensity = 0;
	int minPix = pR->area() * 0.5;

#ifndef USE_OPENCV2X
	cuda::calcHist(gMat2, gHist);
	gHist.download(cHist);

	for (int i = cHist.cols - 1; i > 0; i--)
	{
		intensity += cHist.at<int>(0, i);
		if (intensity > minPix)
		{
			return (255.0f - i) / 255.0f;
		}
	}
#else
	int channels[] =
	{	0};
	int bin_num = 256;
	int bin_nums[] =
	{	bin_num};
	float range[] =
	{	0, 256};
	const float *ranges[] =
	{	range};
	Mat cMat;
	gMat2.download(cMat);
	cv::calcHist(&cMat, 1, channels, cv::Mat(), cHist, 1, bin_nums, ranges);

	for (int i = cHist.rows-1; i > 0; i--)
	{
		intensity += cHist.at<int>(i, 0);
		if(intensity > minPix)
		{
			return (255.0f - i)/255.0f;
		}
	}
#endif

	return -1.0;
}

bool _DepthVisionBase::draw(void)
{
	IF_F(!this->_VisionBase::draw());
	NULL_F(m_pDepthWin);
	NULL_F(m_pDepth);
	IF_F(m_pDepth->empty());

	Frame* pFrame = m_pDepthWin->getFrame();
	pFrame->update(m_pDepth);

	return true;
}


}
