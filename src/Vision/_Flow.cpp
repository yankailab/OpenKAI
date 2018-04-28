/*
 * _Flow.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_Flow.h"

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

	m_pFarn = cuda::FarnebackOpticalFlow::create();
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
	pGray = m_pVision->Gray();
	NULL_(pGray);
	IF_(pGray->bEmpty());

	pNextFrame = m_pGrayFrames->getLastFrame();
	if(*pGray <= *pNextFrame)return;

	m_pGrayFrames->updateFrameIndex();
	pNextFrame = m_pGrayFrames->getLastFrame();
	pPrevFrame = m_pGrayFrames->getPrevFrame();

	*pNextFrame = pGray->resize(m_width, m_height);
//	pPrev = pPrevFrame->getGMat();
//	pNext = pNextFrame->getGMat();
	//TODO:

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
	IF_F(pGray->bEmpty());

	if(!bFrame)
	{
		pFrameA = m_pGrayFrames->getPrevFrame();
		*pFrameA = pGray->resize(m_width, m_height);
		return true;
	}

	pFrameA = m_pGrayFrames->getPrevFrame();
	pFrameB = m_pGrayFrames->getLastFrame();
	*pFrameB = pGray->resize(m_width, m_height);

	IF_F(pFrameA->bEmpty());
	IF_F(pFrameB->bEmpty());
//	IF_F(pFrameA->getGMat()->size() != pFrameB->getGMat()->size());

//	m_pFarn->calc(*pFrameA->getGMat(), *pFrameB->getGMat(), m_gFlow);

	//TODO

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
//	*pFrame = m_gFlow;
//TODO:

	return true;
}

}
