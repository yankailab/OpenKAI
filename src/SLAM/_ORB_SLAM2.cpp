/*
 * _ORB_SLAM2.cpp
 *
 *  Created on: Jul 18, 2017
 *      Author: yankai
 */

#include "_ORB_SLAM2.h"

#ifdef USE_ORB_SLAM2

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

	m_vT.init();
	m_bTracking = false;
	m_bViewer = false;

	m_mRwc = Mat(3,3,CV_32F);
	m_mTwc = Mat(3,1,CV_32F);
}

_ORB_SLAM2::~_ORB_SLAM2()
{
	if (m_pOS)
	{
		m_pOS->Shutdown();
		delete m_pOS;
	}

	DEL(m_pFrame);
}

bool _ORB_SLAM2::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, width);
	KISSm(pK, height);
	KISSm(pK, bViewer);

	string fileVocabulary = "";
	string fileSetting = "";

	F_INFO(pK->v("fileVocabulary", &fileVocabulary));
	F_INFO(pK->v("fileSetting", &fileSetting));

	ifstream ifs;
	ifs.open(fileSetting.c_str(), std::ios::in);
	IF_Fl(!ifs,"setting file not found");

	// Create SLAM system. It initializes all system threads and gets ready to process frames.
	m_pOS = new ORB_SLAM2::System(fileVocabulary, fileSetting,
			ORB_SLAM2::System::MONOCULAR, m_bViewer);

	m_pFrame = new Frame();
	m_tStartup = 0;

	//link
	string iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));

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

bool _ORB_SLAM2::bTracking(void)
{
	return m_bTracking;
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
	static const double usecBase = 1.0 / ((double) USEC_1SEC);

	NULL_(m_pOS);
	NULL_(m_pVision);
	Frame* pGray = m_pVision->Gray();
	NULL_(pGray);
	IF_(pGray->bEmpty());
	*m_pFrame = pGray->resize(m_width, m_height);

	uint64_t tNow = getTimeUsec();
	if (m_tStartup <= 0)
	{
		m_tStartup = tNow;
	}

	double t = ((double) (tNow - m_tStartup)) * usecBase;

	m_pose = m_pOS->TrackMonocular(*m_pFrame->m(), t);
	if (m_pose.empty())
	{
		m_bTracking = false;

		//TODO: if lost too long reset the system

		return;
	}

	m_bTracking = true;

	m_mRwc = m_pose.rowRange(0,3).colRange(0,3).t();
	m_mTwc = -m_mRwc*m_pose.rowRange(0,3).col(3);

	m_vT.x = (double) m_mTwc.at<float>(0); //Right
	m_vT.y = (double) m_mTwc.at<float>(1); //Down
	m_vT.z = (double) m_mTwc.at<float>(2); //Front

	Eigen::Matrix3f mRwc;
	cv2eigen(m_mRwc,mRwc);
	Eigen::Quaternionf qW;
	qW = mRwc;

	m_vQ.x = (double) qW.x();
	m_vQ.y = (double) qW.y();
	m_vQ.z = (double) qW.z();
	m_vQ.w = (double) qW.w();

	cout.precision(5);
	cout << "vT: " << "  " << fixed << m_vT.x << "  " << fixed << m_vT.y << "  " << fixed << m_vT.z << "  " <<
			"Q: "  << "  " << fixed << m_vQ.x << "  " << fixed << m_vQ.y << "  " << fixed << m_vQ.z << "  " << fixed << m_vQ.w << endl;
}

bool _ORB_SLAM2::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;

	string msg;
	pWin->tabNext();

	if (m_bTracking)
	{
		msg = "Tracking";
	}
	else
	{
		msg = "Tracking lost";
	}
	pWin->addMsg(&msg);

	msg = "Global pos: x=" + f2str(m_vT.x) + ", y=" + f2str(m_vT.y) + ", z=" + f2str(m_vT.z);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

}

#endif
