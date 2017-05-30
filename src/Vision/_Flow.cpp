/*
 * _Flow.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "../Vision/_Flow.h"

namespace kai
{

_Flow::_Flow()
{
	m_width = 640;
	m_height = 480;

	m_pVision = NULL;
	m_pGrayFrames = NULL;

}

_Flow::~_Flow()
{
}

bool _Flow::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,width);
	KISSm(pK,height);

#ifndef USE_OPENCV4TEGRA
	m_pFarn = cuda::FarnebackOpticalFlow::create();
#else
	m_pFarn = superres::createOptFlow_Farneback_GPU();
#endif

	m_pGrayFrames = new FrameGroup();
	m_pGrayFrames->init(2);

	return true;
}

bool _Flow::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_INFO(pK->v("_VisionBase",&iName));
	m_pVision = (_VisionBase*)(pK->root()->getChildInstByName(&iName));

	return true;
}

bool _Flow::start(void)
{
	NULL_T(m_pVision);

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Flow::update(void)
{

	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _Flow::detect(void)
{
	Frame* pGray;
	Frame* pNextFrame;
	Frame* pPrevFrame;
	GpuMat* pPrev;
	GpuMat* pNext;

	NULL_(m_pVision);
	pGray = m_pVision->gray();
	NULL_(pGray);
	IF_(pGray->empty());

	pNextFrame = m_pGrayFrames->getLastFrame();
	if(pGray->getFrameID() <= pNextFrame->getFrameID())return;

	m_pGrayFrames->updateFrameIndex();
	pNextFrame = m_pGrayFrames->getLastFrame();
	pPrevFrame = m_pGrayFrames->getPrevFrame();

	pNextFrame->getResizedOf(pGray, m_width, m_height);
	pPrev = pPrevFrame->getGMat();
	pNext = pNextFrame->getGMat();

	if(pPrev->empty())return;
	if(pNext->empty())return;
	if(pPrev->size() != pNext->size())return;

	m_pFarn->calc(*pPrev, *pNext, m_gFlow);

}

bool _Flow::addFrame(bool bFrame, Frame* pGray)
{
	Frame* pFrameA;
	Frame* pFrameB;

	NULL_F(pGray);
	IF_F(pGray->empty());

	if(!bFrame)
	{
		pFrameA = m_pGrayFrames->getPrevFrame();
		pFrameA->getResizedOf(pGray, m_width, m_height);
		return true;
	}

	pFrameA = m_pGrayFrames->getPrevFrame();
	pFrameB = m_pGrayFrames->getLastFrame();
	pFrameB->getResizedOf(pGray, m_width, m_height);

	IF_F(pFrameA->empty());
	IF_F(pFrameB->empty());
	IF_F(pFrameA->getGMat()->size() != pFrameB->getGMat()->size());

	m_pFarn->calc(*pFrameA->getGMat(), *pFrameB->getGMat(), m_gFlow);

	return true;
}

GpuMat* _Flow::flowMat(void)
{
	return &m_gFlow;
}

bool _Flow::draw(void)
{
	IF_F(!this->BASE::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();

	IF_F(m_gFlow.empty());
	pFrame->update(&m_gFlow);

	return true;
}

}
