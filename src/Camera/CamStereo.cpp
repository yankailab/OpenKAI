/*
 * CamStereo.cpp
 *
 *  Created on: Aug 25, 2015
 *      Author: yankai
 */

#include "CamStereo.h"

namespace kai
{

CamStereo::CamStereo()
{
	m_disparity = 16;
}

CamStereo::~CamStereo()
{
	// TODO Auto-generated destructor stub
}

bool CamStereo::init(int disparity)
{
	m_disparity = disparity;

	m_pBM = cuda::createStereoBM(m_disparity);
	m_pBP = cuda::createStereoBeliefPropagation(m_disparity);
	m_pCSBP = cv::cuda::createStereoConstantSpaceBP(m_disparity);
}

void CamStereo::detect(CamFrame* pLeft, CamFrame* pRight, CamFrame* pDepth)
{
	GpuMat* pL = pLeft->getGMat();
	GpuMat* pR = pRight->getGMat();
	GpuMat* pD = pDepth->getGMat();

	//BM
	m_pBM->compute(*pL, *pR, *pD);

//	m_pBP->compute(*pL, *pR, *pD);
//	m_pCSBP->compute(*pLeft->m_pNext, *pRight->m_pNext, *pDepth->m_pNext);

}

void CamStereo::detect(CamFrame* pLRsbs, CamFrame* pDepth)
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

} /* namespace kai */
