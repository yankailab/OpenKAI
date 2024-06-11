/*
 * _DenseFlow.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_DenseFlow.h"

namespace kai
{

	_DenseFlow::_DenseFlow()
	{
		m_pVision = NULL;
		m_pGrayFrames = NULL;
		m_w = 640;
		m_h = 480;

		m_nHistLev = 128;
		m_vRange.x = 0.0;
		m_vRange.y = 1.0;
		m_minHistD = 0.25;
	}

	_DenseFlow::~_DenseFlow()
	{
	}

	bool _DenseFlow::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("w", &m_w);
		pK->v("h", &m_h);
		m_gFlow = GpuMat(m_h, m_w, CV_32FC2);

		m_pGrayFrames = new FrameGroup();
		m_pGrayFrames->init(2);
		m_pFarn = cuda::FarnebackOpticalFlow::create();

		string n = "";
		F_INFO(pK->v("_VisionBase", &n));
		m_pVision = (_VisionBase *)(pK->getInst(n));

		return true;
	}

	bool _DenseFlow::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _DenseFlow::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			detect();

			m_pT->autoFPSto();
		}
	}

	void _DenseFlow::detect(void)
	{
		NULL_(m_pVision);
		Frame *pGray = m_pVision->getFrameRGB();
		NULL_(pGray);
		IF_(pGray->bEmpty());

		Frame *pNextFrame = m_pGrayFrames->getLastFrame();
		IF_(pGray->tStamp() <= pNextFrame->tStamp());
		Frame *pPrevFrame = m_pGrayFrames->getPrevFrame();

		m_pGrayFrames->updateFrameIndex();

		pNextFrame->copy(pGray->resize(m_w, m_h).cvtColor(COLOR_BGR2GRAY));
		GpuMat *pPrev = pPrevFrame->gm();
		GpuMat *pNext = pNextFrame->gm();

		IF_(pPrev->empty());
		IF_(pNext->empty());
		IF_(pPrev->size() != pNext->size());

		m_pFarn->calc(*pPrev, *pNext, m_gFlow);

		Mat mFlow;
		m_gFlow.download(mFlow);
		cv::split(mFlow, m_pFlow);
	}

	vDouble2 _DenseFlow::vFlow(vDouble4 *pROI)
	{
		vDouble2 vF;
		vF.clear();
		if (!pROI)
			return vF;

		vInt4 iR;
		iR.x = pROI->x * m_w;
		iR.y = pROI->y * m_h;
		iR.z = pROI->z * m_w;
		iR.w = pROI->w * m_h;

		if (iR.x < 0)
			iR.x = 0;
		if (iR.y < 0)
			iR.y = 0;

		if (iR.z > m_w)
			iR.z = m_w;
		if (iR.w > m_h)
			iR.w = m_h;

		if (iR.z < iR.x)
			iR.z = iR.x;
		if (iR.w < iR.y)
			iR.w = iR.y;

		return vFlow(&iR);
	}

	vDouble2 _DenseFlow::vFlow(vInt4 *pROI)
	{
		vDouble2 vF;
		vF.clear();

		IF__(!pROI, vF);
		IF__(m_pFlow[0].empty(), vF);
		IF__(m_pFlow[1].empty(), vF);

		vector<int> vHistLev = {m_nHistLev};
		vector<float> vRange = {(float)m_vRange.x, (float)m_vRange.y};
		vector<int> vChannel = {0};

		Rect roi = bb2Rect(*pROI);
		double nBase = (m_vRange.y - m_vRange.x) / (double)m_nHistLev;
		float nMinHist = (float)(m_minHistD * pROI->area());
		Mat mHist;
		int i;

		Mat mRoiX = m_pFlow[0](roi);
		vector<Mat> vRoiX = {mRoiX};
		cv::calcHist(vRoiX, vChannel, Mat(),
					 mHist, vHistLev, vRange,
					 true //accumulate
		);
		for (i = 0; i < m_nHistLev; i++)
		{
			if (mHist.at<float>(i) >= nMinHist)
				break;
		}
		vF.x = m_vRange.x + ((double)i) * nBase;

		Mat mRoiY = m_pFlow[1](roi);
		vector<Mat> vRoiY = {mRoiY};
		cv::calcHist(vRoiY, vChannel, Mat(),
					 mHist, vHistLev, vRange,
					 true //accumulate
		);
		for (i = 0; i < m_nHistLev; i++)
		{
			if (mHist.at<float>(i) >= nMinHist)
				break;
		}
		vF.y = m_vRange.x + ((double)i) * nBase;

		return vF;
	}

	void _DenseFlow::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);

		IF_(m_pFlow[0].empty());
		IF_(m_pFlow[1].empty());

		Mat mF;
		drawOpticalFlow(m_pFlow[0], m_pFlow[1], mF, 10);
		imshow(this->getName(), mF);
	}

	bool _DenseFlow::isFlowCorrect(Point2f u)
	{
		return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
	}

	Vec3b _DenseFlow::computeColor(float fx, float fy)
	{
		static bool first = true;

		// relative lengths of color transitions:
		// these are chosen based on perceptual similarity
		// (e.g. one can distinguish more shades between red and yellow
		//  than between yellow and green)
		const int RY = 15;
		const int YG = 6;
		const int GC = 4;
		const int CB = 11;
		const int BM = 13;
		const int MR = 6;
		const int NCOLS = RY + YG + GC + CB + BM + MR;
		static Vec3i colorWheel[NCOLS];

		if (first)
		{
			int k = 0;

			for (int i = 0; i < RY; ++i, ++k)
				colorWheel[k] = Vec3i(255, 255 * i / RY, 0);

			for (int i = 0; i < YG; ++i, ++k)
				colorWheel[k] = Vec3i(255 - 255 * i / YG, 255, 0);

			for (int i = 0; i < GC; ++i, ++k)
				colorWheel[k] = Vec3i(0, 255, 255 * i / GC);

			for (int i = 0; i < CB; ++i, ++k)
				colorWheel[k] = Vec3i(0, 255 - 255 * i / CB, 255);

			for (int i = 0; i < BM; ++i, ++k)
				colorWheel[k] = Vec3i(255 * i / BM, 0, 255);

			for (int i = 0; i < MR; ++i, ++k)
				colorWheel[k] = Vec3i(255, 0, 255 - 255 * i / MR);

			first = false;
		}

		const float rad = sqrt(fx * fx + fy * fy);
		const float a = atan2(-fy, -fx) / (float)CV_PI;

		const float fk = (a + 1.0f) / 2.0f * (NCOLS - 1);
		const int k0 = static_cast<int>(fk);
		const int k1 = (k0 + 1) % NCOLS;
		const float f = fk - k0;

		Vec3b pix;

		for (int b = 0; b < 3; b++)
		{
			const float col0 = colorWheel[k0][b] / 255.0f;
			const float col1 = colorWheel[k1][b] / 255.0f;

			float col = (1 - f) * col0 + f * col1;

			if (rad <= 1)
				col = 1 - rad * (1 - col); // increase saturation with radius
			else
				col *= .75; // out of range

			pix[2 - b] = static_cast<uchar>(255.0 * col);
		}

		return pix;
	}

	void _DenseFlow::drawOpticalFlow(const Mat_<float> &flowx, const Mat_<float> &flowy, Mat &dst, float maxmotion)
	{
		dst.create(flowx.size(), CV_8UC3);
		dst.setTo(Scalar::all(0));

		// determine motion range:
		float maxrad = maxmotion;

		if (maxmotion <= 0)
		{
			maxrad = 1;
			for (int y = 0; y < flowx.rows; ++y)
			{
				for (int x = 0; x < flowx.cols; ++x)
				{
					Point2f u(flowx(y, x), flowy(y, x));

					if (!isFlowCorrect(u))
						continue;

					maxrad = max(maxrad, sqrt(u.x * u.x + u.y * u.y));
				}
			}
		}

		for (int y = 0; y < flowx.rows; ++y)
		{
			for (int x = 0; x < flowx.cols; ++x)
			{
				Point2f u(flowx(y, x), flowy(y, x));

				if (isFlowCorrect(u))
					dst.at<Vec3b>(y, x) = computeColor(u.x / maxrad, u.y / maxrad);
			}
		}
	}

}
