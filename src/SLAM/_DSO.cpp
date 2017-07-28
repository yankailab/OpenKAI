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
	m_frameID = 0;

}

_DSO::~_DSO()
{
	DEL(m_pFrame);
	DEL(m_pDSO);
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
	m_pDSO = new DSOwrap();
	m_pDSO->m_bViewer = true;
	m_pDSO->m_width = m_width;
	m_pDSO->m_height = m_height;
	m_pDSO->init();

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
	m_pDSO->reset();
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
	if(m_pDSO->bInitFailed())
	{
		LOG_I("DSO init failed");
		reset();
		return;
	}

	if(m_pDSO->bLost())
	{
		LOG_I("DSO lost");
		return;
	}

	NULL_(m_pVision);
	Frame* pGray = m_pVision->gray();
	NULL_(pGray);
	IF_(pGray->empty());
	m_pFrame->getResizedOf(pGray, m_width, m_height);

	static const double usecBase = 1.0 / ((double) USEC_1SEC);
	uint64_t tNow = get_time_usec();
	double t = ((double)tNow) * usecBase;

	static Mat fImg;
	m_pFrame->getCMat()->convertTo(fImg, CV_32FC1);
	m_pDSO->addFrame(fImg.data,t);

//	m_pDSO->addFrame(m_pFrame->getCMat()->data);
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
