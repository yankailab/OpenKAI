/*
 * _DSO.cpp
 *
 *  Created on: Jul 26, 2017
 *      Author: yankai
 */

#include "_DSO.h"

#ifdef USE_DSO

namespace kai
{

_DSO::_DSO()
{
	m_width = 640;
	m_height = 360;

	m_pVision = NULL;
	m_pFrame = NULL;
	m_bViewer = false;

	m_pDSO = NULL;
	m_pPangolin = NULL;
	m_pOW = NULL;
	m_pImg = NULL;
	m_frameID = 0;

}

_DSO::~_DSO()
{
	DEL(m_pFrame);
	DEL(m_pDSO);
	DEL(m_pPangolin);
	DEL(m_pOW);
	DEL(m_pImg);
}

bool _DSO::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, width);
	KISSm(pK, height);
	KISSm(pK, bViewer);

	m_pFrame = new Frame();
	reset();

	m_pImg = new dso::ImageAndExposure(m_width, m_height, 0);

	return true;
}

bool _DSO::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _DSO::start(void)
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

void _DSO::reset(void)
{
	DEL(m_pDSO);

	m_pDSO = new dso::FullSystem();
//	m_pDSO->linearizeOperation = true;
//	m_pDSO->setGammaFunction(reader->getPhotometricGamma());

	if(!m_pOW)
	{
		m_pOW = new dso::IOWrap::DSOoutputWrapper();
	}
	m_pOW->reset();
	m_pDSO->outputWrapper.push_back(m_pOW);

	if(m_bViewer)
    {
		if(!m_pPangolin)
		{
	        m_pPangolin = new dso::IOWrap::PangolinDSOViewer(1280,720,false);//(wG[0],hG[0], false);
		}
    	m_pPangolin->reset();
        m_pDSO->outputWrapper.push_back(m_pPangolin);
    }

	m_frameID = 0;

}

void _DSO::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _DSO::detect(void)
{
	NULL_(m_pDSO);
	if(m_pDSO->initFailed)
	{
		reset();
		return;
	}

	if(m_pDSO->isLost)
	{
		LOG_I("DSO lost");
		return;
	}

	NULL_(m_pVision);
	Frame* pGray = m_pVision->gray();
	NULL_(pGray);
	IF_(pGray->empty());
	m_pFrame->getResizedOf(pGray, m_width, m_height);

	m_pDSO->addActiveFrame();

}

bool _DSO::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;
	pWin->tabNext();

//	msg = "Global pos: x=" + f2str(m_vT.x) + ", y=" + f2str(m_vT.y) + ", z=" + f2str(m_vT.z);
//	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

}

#endif
