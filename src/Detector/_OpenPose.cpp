/*
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */
#include "_OpenPose.h"

namespace kai
{

_OpenPose::_OpenPose()
{
	m_nW = 368;
	m_nH = 368;
	m_bSwapRB = false;
	m_vMean.init();
	m_scale = 1.0 / 255.0;
	m_thr = 0.1;

	m_iBackend = dnn::DNN_BACKEND_OPENCV;
	m_iTarget = dnn::DNN_TARGET_CPU;
}

_OpenPose::~_OpenPose()
{
}

bool _OpenPose::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nW", &m_nW);
	pK->v("nH", &m_nH);
	pK->v("iBackend", &m_iBackend);
	pK->v("iTarget", &m_iTarget);
	pK->v("bSwapRB", &m_bSwapRB);
	pK->v("scale", &m_scale);
	pK->v("meanB", &m_vMean.x);
	pK->v("meanG", &m_vMean.y);
	pK->v("meanR", &m_vMean.z);

	m_net = readNetFromCaffe(m_modelFile, m_trainedFile);
	IF_Fl(m_net.empty(), "read Net failed");

	m_net.setPreferableBackend(m_iBackend);
	m_net.setPreferableTarget(m_iTarget);

	return true;
}

bool _OpenPose::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _OpenPose::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		updateObj();

		if (m_bGoSleep)
		{
			m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _OpenPose::check(void)
{
	NULL__(m_pVision, -1);
	Frame* pBGR = m_pVision->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);
	IF__(pBGR->tStamp() <= m_fBGR.tStamp(), -1);

	return 0;
}

bool _OpenPose::detect(void)
{
	IF_F(check() < 0);

	Frame* pBGR = m_pVision->BGR();
	m_fBGR.copy(*pBGR);
	Mat mIn = *m_fBGR.m();

	m_blob = blobFromImage(mIn, m_scale, Size(m_nW, m_nH),
			Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);
	m_net.setInput(m_blob);

	Mat output = m_net.forward();

	int H = output.size[2];
	int W = output.size[3];

	m_mDebug = mIn.clone();

	// find the position of the body parts
	vector<Point> points(OP_N_POINTS);
	for (int n = 0; n < OP_N_POINTS; n++)
	{
		// Probability map of corresponding body's part.
		Mat probMap(H, W, CV_32F, output.ptr(0, n));

		Point2f p(-1, -1);
		Point maxLoc;
		double prob;
		minMaxLoc(probMap, 0, &prob, 0, &maxLoc);
		if (prob > m_thr)
		{
			p = maxLoc;
			p.x *= (float) mIn.cols / W;
			p.y *= (float) mIn.rows / H;

			circle(m_mDebug, cv::Point((int) p.x, (int) p.y), 8,
					Scalar(0, 255, 255), -1);
			cv::putText(m_mDebug, cv::format("%d", n),
					cv::Point((int) p.x, (int) p.y), cv::FONT_HERSHEY_COMPLEX,
					1, cv::Scalar(0, 0, 255), 2);

		}
		points[n] = p;
	}

	int nPairs = sizeof(POSE_PAIRS) / sizeof(POSE_PAIRS[0]);

	for (int n = 0; n < nPairs; n++)
	{
		// lookup 2 connected body/hand parts
		Point2f partA = points[POSE_PAIRS[n][0]];
		Point2f partB = points[POSE_PAIRS[n][1]];

		if (partA.x <= 0 || partA.y <= 0 || partB.x <= 0 || partB.y <= 0)
			continue;

		line(m_mDebug, partA, partB, Scalar(0, 255, 255), 8);
		circle(m_mDebug, partA, 8, Scalar(0, 0, 255), -1);
		circle(m_mDebug, partB, 8, Scalar(0, 0, 255), -1);
	}

	return true;
}

bool _OpenPose::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	if (!m_mDebug.empty())
	{
		imshow("Pose", m_mDebug);
	}

	return true;
}

}
