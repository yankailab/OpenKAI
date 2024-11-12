/*
 * _SLAMbase.cpp
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#include "_SLAMbase.h"

namespace kai
{

	_SLAMbase::_SLAMbase()
	{
		m_pV = nullptr;
		m_vSize.set(640, 360);
		m_bTracking = false;
	}

	_SLAMbase::~_SLAMbase()
	{
	}

	int _SLAMbase::init(void *pKiss)
	{
		CHECK_(this->_NavBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vSize", &m_vSize);

		return OK_OK;
	}

	int _SLAMbase::link(void)
	{
		CHECK_(this->_NavBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));

		return OK_OK;
	}

	int _SLAMbase::check(void)
	{
		NULL__(m_pV, OK_ERR_NULLPTR);
		NULL__(m_pV->getFrameRGB(), OK_ERR_NOT_READY);

		return this->_NavBase::check();
	}

	int _SLAMbase::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	bool _SLAMbase::bTracking(void)
	{
		return m_bTracking;
	}

	void _SLAMbase::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			detect();

			m_pT->autoFPSto();
		}
	}

	void _SLAMbase::detect(void)
	{
		IF_(check() != OK_OK);

		static const double usecBase = 1.0 / ((double)SEC_2_USEC);

		Mat mGray;
		m_pV->getFrameRGB()->m()->copyTo(mGray);
		IF_(mGray.empty());

//		m_pose = m_pOS->TrackMonocular(mGray, t);
		// if (m_pose.empty())
		// {
		// 	m_bTracking = false;

		// 	//TODO: if lost too long reset the system

		// 	return;
		// }

		// m_bTracking = true;
/*
		m_mRwc = m_pose.rowRange(0, 3).colRange(0, 3).t();
		m_mTwc = -m_mRwc * m_pose.rowRange(0, 3).col(3);

		m_vT.x = (double)m_mTwc.at<float>(0); //Right
		m_vT.y = (double)m_mTwc.at<float>(1); //Down
		m_vT.z = (double)m_mTwc.at<float>(2); //Front

		Eigen::Matrix3f mRwc;
		cv2eigen(m_mRwc, mRwc);
		Eigen::Quaternionf qW;
		qW = mRwc;

		m_vQ.x = (double)qW.x();
		m_vQ.y = (double)qW.y();
		m_vQ.z = (double)qW.z();
		m_vQ.w = (double)qW.w();

		cout.precision(5);
		cout << "vT: "
			 << "  " << fixed << m_vT.x << "  " << fixed << m_vT.y << "  " << fixed << m_vT.z << "  "
			 << "Q: "
			 << "  " << fixed << m_vQ.x << "  " << fixed << m_vQ.y << "  " << fixed << m_vQ.z << "  " << fixed << m_vQ.w << endl;
*/
	}

	void _SLAMbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_NavBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		string msg;
		if (m_bTracking)
		{
			msg = "Tracking";
		}
		else
		{
			msg = "Tracking lost";
		}
		pC->addMsg(msg);

		msg = "Global pos: x=" + f2str(m_vT.x) + ", y=" + f2str(m_vT.y) + ", z=" + f2str(m_vT.z);
		pC->addMsg(msg);
	}

}
