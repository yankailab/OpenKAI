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
	m_disparity = 128;

}

CamStereo::~CamStereo()
{
	// TODO Auto-generated destructor stub
}

bool CamStereo::init(void)
{
	m_pBM = cuda::createStereoBM(m_disparity);
	m_pBP = cuda::createStereoBeliefPropagation(m_disparity);
	m_pCSBP = cv::cuda::createStereoConstantSpaceBP(m_disparity);

//	namedWindow("Left");
//	namedWindow("Right");
//	namedWindow("Stereo");

}

void CamStereo::detect(CamFrame* pLeft, CamFrame* pRight, CamFrame* pDepth)
{
	GpuMat* pL = pLeft->getGMat();
	GpuMat* pR = pRight->getGMat();
	GpuMat* pD = pDepth->getGMat();

	//BM
	m_pBM->compute(*pL, *pR, *pD);

//	m_pBP->compute(*pLeft->m_pNext, *pRight->m_pNext, *pDepth->m_pNext);

//	m_pCSBP->compute(*pLeft->m_pNext, *pRight->m_pNext, *pDepth->m_pNext);


}

} /* namespace kai */
