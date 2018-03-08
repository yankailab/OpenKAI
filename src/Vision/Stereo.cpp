/*
 * Stereo.cpp
 *
 *  Created on: Aug 25, 2015
 *      Author: yankai
 */

#include "Stereo.h"

#ifdef USE_CUDASTEREO

namespace kai
{

Stereo::Stereo()
{
	m_disparity = 16;
}

Stereo::~Stereo()
{
}

bool Stereo::init(int disparity)
{
	m_disparity = disparity;

	m_pBM = cuda::createStereoBM(m_disparity);
	m_pBP = cuda::createStereoBeliefPropagation(m_disparity);
	m_pCSBP = cv::cuda::createStereoConstantSpaceBP(m_disparity);
	return true;
}

void Stereo::detect(Frame* pLeft, Frame* pRight, Frame* pDepth)
{
	GpuMat* pL = pLeft->getGMat();
	GpuMat* pR = pRight->getGMat();
	GpuMat* pD = pDepth->getGMat();

#ifndef USE_OPENCV2X
	//BM
	m_pBM->compute(*pL, *pR, *pD);

#endif
//	m_pBP->compute(*pL, *pR, *pD);
//	m_pCSBP->compute(*pLeft->m_pNext, *pRight->m_pNext, *pDepth->m_pNext);

}

void Stereo::detect(Frame* pLRsbs, Frame* pDepth)
{
	GpuMat* pLR = pLRsbs->getGMat();
	GpuMat GDepth;

	int width = pLR->cols/2;
	int height = pLR->rows;

	GpuMat mL = (*pLR)(cv::Rect(0, 0, width, height));
	GpuMat mR = (*pLR)(cv::Rect(width, 0, width, height));

	m_pBM->compute(mL, mR, GDepth);
//	m_pBP->compute(mL, mR, GDepth);
//	m_pCSBP->compute(mL, mR, GDepth);
	pDepth->update(&GDepth);

}
}
#endif
