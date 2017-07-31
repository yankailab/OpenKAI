/*
 * _LSD_SLAM.cpp
 *
 *  Created on: Jul 26, 2017
 *      Author: yankai
 */

#include "_LSD_SLAM.h"

#ifdef USE_LSD_SLAM

namespace kai
{

_LSD_SLAM::_LSD_SLAM()
{
	m_width = 640;
	m_height = 480;

	m_pVision = NULL;
	m_pFrame = NULL;
	m_pLSD = NULL;
	m_bViewer = false;
	m_fCamConfig = "";
}

_LSD_SLAM::~_LSD_SLAM()
{
	DEL(m_pFrame);
	DEL(m_pLSD);
}

bool _LSD_SLAM::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, width);
	KISSm(pK, height);
	KISSm(pK, bViewer);
	KISSm(pK, fCamConfig);

	m_pFrame = new Frame();

	m_pLSD = new LSDSLAM();
	if(!m_pLSD->init((void*)m_fCamConfig.c_str()))
	{
		LOG_E("lsd_slam init failed");
		return false;
	}

	return true;
}

bool _LSD_SLAM::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _LSD_SLAM::start(void)
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

void _LSD_SLAM::reset(void)
{
}

void _LSD_SLAM::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _LSD_SLAM::detect(void)
{
	NULL_(m_pLSD);
	NULL_(m_pVision);
	Frame* pGray = m_pVision->gray();
	NULL_(pGray);
	IF_(pGray->empty());

	static const double usecBase = 1.0 / ((double) USEC_1SEC);
	uint64_t tNow = get_time_usec();
	double t = ((double)tNow) * usecBase;

	m_pFrame->getResizedOf(pGray, m_width, m_height);

	if(!m_pLSD->update((void*)m_pFrame->getCMat(),t))
	{
		m_pLSD->reset();
	}
}

bool _LSD_SLAM::draw(void)
{
	m_pLSD->draw();

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
