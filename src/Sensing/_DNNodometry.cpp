/*
 * _DNNodometry.cpp
 *
 *  Created on: June 8, 2017
 *      Author: yankai
 */

#include "_DNNodometry.h"

namespace kai
{

_DNNodometry::_DNNodometry()
{
	m_pCam = NULL;
	m_pFrames = NULL;
	m_width = 398;
	m_height = 224;
	m_bCrop = false;

	m_vT.init();
	m_vR.init();
}

_DNNodometry::~_DNNodometry()
{
}

bool _DNNodometry::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,bCrop);
	if (m_bCrop != 0)
	{
		F_ERROR_F(pK->v("cropX", &m_cropBB.x));
		F_ERROR_F(pK->v("cropY", &m_cropBB.y));
		F_ERROR_F(pK->v("cropW", &m_cropBB.width));
		F_ERROR_F(pK->v("cropH", &m_cropBB.height));
	}

	m_pFrames = new FrameGroup();
	m_pFrames->init(2);

	return true;
}

bool _DNNodometry::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_Camera", &iName));
	m_pCam = (_Camera*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _DNNodometry::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _DNNodometry::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _DNNodometry::detect(void)
{
	Frame* pGray;
	Frame* pNextF;
	Frame* pPrevF;
	GpuMat* pPrevG;
	GpuMat* pNextG;

	NULL_(m_pCam);
	pGray = m_pCam->gray();
	NULL_(pGray);
	IF_(pGray->empty());

	pNextF = m_pFrames->getLastFrame();
	if(pGray->getFrameID() <= pNextF->getFrameID())return;

	m_pFrames->updateFrameIndex();
	pNextF = m_pFrames->getLastFrame();
	pPrevF = m_pFrames->getPrevFrame();

	pNextF->getResizedOf(pGray, m_width, m_height);
	pPrevG = pPrevF->getGMat();
	pNextG = pNextF->getGMat();

	if(pPrevG->empty())return;
	if(pNextG->empty())return;
	if(pPrevG->size() != pNextG->size())return;

}

void _DNNodometry::d(vDouble3* pT, vDouble3* pR)
{
	if(pT)
	{
		*pT = m_vT;
		m_vT.init();
	}

	if(pR)
	{
		*pR = m_vR;
		m_vR.init();
	}
}

bool _DNNodometry::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	IF_F(pMat->empty());

	return true;
}

}
