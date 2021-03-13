/*
 * _ORB_SLAM.cpp
 *
 *  Created on: Jul 18, 2017
 *      Author: yankai
 */

#include "_ORB_SLAM.h"

#ifdef USE_OPENCV
#ifdef USE_ORB_SLAM

namespace kai
{

_ORB_SLAM::_ORB_SLAM()
{
    m_vSize.init(640,360);
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

_ORB_SLAM::~_ORB_SLAM()
{
	if (m_pOS)
	{
		m_pOS->Shutdown();
		delete m_pOS;
	}

	DEL(m_pFrame);
}

bool _ORB_SLAM::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

    pK->v("vSize", &m_vSize);
    pK->v("bViewer", &m_bViewer);

	string fileVocabulary = "";
	string fileSetting = "";

	F_INFO(pK->v("fileVocabulary", &fileVocabulary));
	F_INFO(pK->v("fileSetting", &fileSetting));

	ifstream ifs;
	ifs.open(fileSetting.c_str(), std::ios::in);
	IF_Fl(!ifs,"setting file not found");

	// Create SLAM system. It initializes all system threads and gets ready to process frames.
	m_pOS = new ORB_SLAM3::System(fileVocabulary, fileSetting,
			ORB_SLAM3::System::MONOCULAR, m_bViewer);

	m_pFrame = new Frame();
	m_tStartup = 0;

	string n = "";
	F_INFO(pK->v("_VisionBase", &n));
	m_pVision = (_VisionBase*) (pK->getInst(n));

	return true;
}

bool _ORB_SLAM::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

bool _ORB_SLAM::bTracking(void)
{
	return m_bTracking;
}

void _ORB_SLAM::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		detect();

		m_pT->autoFPSto();
	}
}

void _ORB_SLAM::detect(void)
{
	static const double usecBase = 1.0 / ((double) USEC_1SEC);

	NULL_(m_pOS);
	NULL_(m_pVision);
	Frame* pGray;// = m_pVision->Gray();
	NULL_(pGray);
	IF_(pGray->bEmpty());
	*m_pFrame = pGray->resize(m_vSize.x, m_vSize.y);

	uint64_t tNow = getApproxTbootUs();
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

void _ORB_SLAM::draw(void)
{
	this->_ModuleBase::draw();

	string msg;
	if (m_bTracking)
	{
		msg = "Tracking";
	}
	else
	{
		msg = "Tracking lost";
	}
	addMsg(msg);

	msg = "Global pos: x=" + f2str(m_vT.x) + ", y=" + f2str(m_vT.y) + ", z=" + f2str(m_vT.z);
	addMsg(msg);
}

}
#endif
#endif
