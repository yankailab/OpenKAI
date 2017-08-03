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
	m_cropBB.x = 0;
	m_cropBB.y = 0;
	m_cropBB.width = 640;
	m_cropBB.height = 480;
	m_resize.x = m_cropBB.width;
	m_resize.y = m_cropBB.height;
	m_pCam = NULL;
	m_pResizeGray = NULL;
	m_pCropGray = NULL;
	m_pLSD = NULL;
	m_bViewer = false;
}

_LSD_SLAM::~_LSD_SLAM()
{
	DEL(m_pResizeGray);
	DEL(m_pLSD);
}

bool _LSD_SLAM::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, bViewer);

	F_INFO(pK->v("resizeW", &m_resize.x));
	F_INFO(pK->v("resizeH", &m_resize.y));
	F_INFO(pK->v("cropW", &m_cropBB.width));
	F_INFO(pK->v("cropH", &m_cropBB.height));

	m_pResizeGray = new Frame();
	m_pCropGray = new Frame();

	return true;
}

bool _LSD_SLAM::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pCam = (_Camera*) (pK->root()->getChildInstByName(&iName));
	IF_F(!m_pCam);

	//find the BB
	IF_F(m_resize.x < m_cropBB.width);
	IF_F(m_resize.y < m_cropBB.height);
	m_cropBB.x = (m_resize.x - m_cropBB.width)/2;
	m_cropBB.y = (m_resize.y - m_cropBB.height)/2;

	//update the K
	vInt2 camSize;
	vDouble2 camScale;

	m_pCam->info(&camSize,NULL,NULL);
	camScale.x = ((double)camSize.x)/((double)m_resize.x);
	camScale.y = ((double)camSize.y)/((double)m_resize.y);
	m_K = m_pCam->m_cameraMat;//*m_pCam->K();

	//scaling
	m_K.at<double>(0, 0) *= camScale.x; //fx
	m_K.at<double>(1, 1) *= camScale.y; //fy
	m_K.at<double>(2, 0) *= camScale.x; //cx
	m_K.at<double>(2, 1) *= camScale.y; //cy

	//crop
	Size imSize(m_resize.x, m_resize.y);
	Mat newK = getOptimalNewCameraMatrix(m_K, m_pCam->m_distCoeffs, imSize, 1, imSize, 0);
	m_K = newK;
	m_K.at<double>(2, 0) -= (double)m_cropBB.x; //cx
	m_K.at<double>(2, 1) -= (double)m_cropBB.y; //cy
	m_K = m_K.t();

	m_pLSD = new LSDSLAM();
	if(!m_pLSD->init(m_cropBB.width, m_cropBB.height, &m_K))
	{
		LOG_E("lsd_slam init failed");
		return false;
	}

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
	NULL_(m_pCam);
	Frame* pGray = m_pCam->gray();
	NULL_(pGray);
	IF_(pGray->empty());

	static const double usecBase = 1.0 / ((double) USEC_1SEC);
	uint64_t tNow = get_time_usec();
	double t = ((double)tNow) * usecBase;

	m_pResizeGray->getResizedOf(pGray, m_resize.x, m_resize.y);
	m_pCropGray->getCropOf(m_pResizeGray,m_cropBB);

	if(!m_pLSD->update((void*)m_pCropGray->getCMat()->data,t))
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
