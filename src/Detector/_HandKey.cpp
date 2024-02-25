/*
 *  Created on: Mar 11, 2021
 *      Author: yankai
 */
#include "_HandKey.h"

namespace kai
{

	_HandKey::_HandKey()
	{
		m_nW = 654;
		m_nH = 368;
		m_bSwapRB = false;
		m_vMean.clear();
		m_scale = 1.0 / 255.0;
		m_thr = 0.01;

		m_iBackend = dnn::DNN_BACKEND_OPENCV;
		m_iTarget = dnn::DNN_TARGET_CPU;
	}

	_HandKey::~_HandKey()
	{
	}

	bool _HandKey::init(void *pKiss)
	{
		IF_F(!this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("nW", &m_nW);
		pK->v("nH", &m_nH);
		pK->v("iBackend", &m_iBackend);
		pK->v("iTarget", &m_iTarget);
		pK->v("bSwapRB", &m_bSwapRB);
		pK->v("scale", &m_scale);
		pK->v("vMean", &m_vMean);

		m_net = readNetFromCaffe(m_fModel, m_fWeight);
		IF_Fl(m_net.empty(), "read Net failed");

		m_net.setPreferableBackend(m_iBackend);
		m_net.setPreferableTarget(m_iTarget);

		return true;
	}

	bool _HandKey::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _HandKey::check(void)
	{
		NULL__(m_pV, -1);
		Frame *pBGR = m_pV->getFrameRGB();
		NULL__(pBGR, -1);
		IF__(pBGR->bEmpty(), -1);
		IF__(pBGR->tStamp() <= m_fRGB.tStamp(), -1);

		return this->_DetectorBase::check();
	}

	void _HandKey::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			detect();

			ON_SLEEP;
			m_pT->autoFPSto();
		}
	}

	void _HandKey::detect(void)
	{
		IF_(check() < 0);

		Frame *pBGR = m_pV->getFrameRGB();
		m_fRGB.copy(*pBGR);
		Mat mIn = *m_fRGB.m();

		m_blob = blobFromImage(mIn, m_scale, Size(m_nW, m_nH), Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);
		m_net.setInput(m_blob);

		Mat output = m_net.forward();

		int H = output.size[2];
		int W = output.size[3];

		m_mDebug = mIn.clone();

		// find the position of the body parts
		vector<Point> points(HANDKEY_N_P);
		for (int n = 0; n < HANDKEY_N_P; n++)
		{
			// Probability map of corresponding body's part.
			Mat probMap(H, W, CV_32F, output.ptr(0, n));

			Point2f p(-1, -1);
			Point maxLoc;
			double prob;
			cv::minMaxLoc(probMap, 0, &prob, 0, &maxLoc);
			if (prob > m_thr)
			{
				p = maxLoc;
				p.x *= (float)mIn.cols / W;
				p.y *= (float)mIn.rows / H;

				circle(m_mDebug, cv::Point((int)p.x, (int)p.y), 8, Scalar(0, 255, 255), -1);
				cv::putText(m_mDebug, cv::format("%d", n),
							cv::Point((int)p.x, (int)p.y), cv::FONT_HERSHEY_COMPLEX,
							1, cv::Scalar(0, 0, 255), 2);
			}
			points[n] = p;
		}

		int nPairs = sizeof(HANDKEY_POSE_PAIRS) / sizeof(HANDKEY_POSE_PAIRS[0]);

		for (int n = 0; n < nPairs; n++)
		{
			// lookup 2 connected body/hand parts
			Point2f partA = points[HANDKEY_POSE_PAIRS[n][0]];
			Point2f partB = points[HANDKEY_POSE_PAIRS[n][1]];

			if (partA.x <= 0 || partA.y <= 0 || partB.x <= 0 || partB.y <= 0)
				continue;

			line(m_mDebug, partA, partB, Scalar(0, 255, 255), 8);
			circle(m_mDebug, partA, 8, Scalar(0, 0, 255), -1);
			circle(m_mDebug, partB, 8, Scalar(0, 0, 255), -1);
		}
	}

	void _HandKey::draw(void* pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(check() < 0);

		if (!m_mDebug.empty())
		{
			imshow("Pose", m_mDebug);
		}
	}

}
