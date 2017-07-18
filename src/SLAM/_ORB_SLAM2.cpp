/*
 * _ORB_SLAM2.cpp
 *
 *  Created on: Jul 18, 2017
 *      Author: yankai
 */

#include "_ORB_SLAM2.h"

namespace kai
{

_ORB_SLAM2::_ORB_SLAM2()
{
	m_width = 640;
	m_height = 360;
	m_tStartup = 0;

	m_pVision = NULL;
	m_pOS = NULL;
	m_pFrame = NULL;

}

_ORB_SLAM2::~_ORB_SLAM2()
{
	if(m_pOS)
	{
		m_pOS->Shutdown();
		delete m_pOS;
	}

	DEL(m_pFrame);
}

bool _ORB_SLAM2::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,width);
	KISSm(pK,height);

	string fileVocabulary = "";
	string fileSetting = "";

	F_INFO(pK->v("fileVocabulary", &fileVocabulary));
	F_INFO(pK->v("fileSetting", &fileSetting));

	// Create SLAM system. It initializes all system threads and gets ready to process frames.
	m_pOS = new ORB_SLAM2::System(fileVocabulary,fileSetting,ORB_SLAM2::System::MONOCULAR,true);

	m_pFrame = new Frame();
	m_tStartup = 0;

	return true;
}

bool _ORB_SLAM2::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_INFO(pK->v("_VisionBase",&iName));
	m_pVision = (_VisionBase*)(pK->root()->getChildInstByName(&iName));

	return true;
}

bool _ORB_SLAM2::start(void)
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

void _ORB_SLAM2::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _ORB_SLAM2::detect(void)
{
	static const double usecBase = 1.0/((double)USEC_1SEC);

	NULL_(m_pOS);
	NULL_(m_pVision);
	Frame* pGray = m_pVision->gray();
	NULL_(pGray);
	IF_(pGray->empty());
	m_pFrame->getResizedOf(pGray,m_width,m_height);

	uint64_t tNow = get_time_usec();
	if(m_tStartup<=0)
	{
		m_tStartup = tNow;
	}

	double t = ((double)(tNow - m_tStartup)) * usecBase;

 	m_pose = m_pOS->TrackMonocular(*m_pFrame->getCMat(), t);

}

bool _ORB_SLAM2::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();



	return true;
}

}
