/*
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */
#include "_DNNtext.h"

namespace kai
{

_DNNtext::_DNNtext()
{
	m_thr = 0.5;
	m_nms = 0.4;
	m_nW = 320;
	m_nH = 320;
	m_vMean.x = 123.68;
	m_vMean.y = 116.78;
	m_vMean.z = 103.94;
	m_scale = 1.0;
}

_DNNtext::~_DNNtext()
{
}

bool _DNNtext::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, thr);
	KISSm(pK, nms);
	KISSm(pK, nW);
	KISSm(pK, nH);
	KISSm(pK, iBackend);
	KISSm(pK, iTarget);
	KISSm(pK, bSwapRB);
	KISSm(pK, scale);
	KISSm(pK, iClassDraw);
	pK->v("meanB", &m_vMean.x);
	pK->v("meanG", &m_vMean.y);
	pK->v("meanR", &m_vMean.z);

	m_net = readNet(m_modelFile);
	IF_Fl(m_net.empty(), "read Net failed");

	m_net.setPreferableBackend(m_iBackend);
	m_net.setPreferableTarget(m_iTarget);

	m_vLayerName.resize(2);
	m_vLayerName[0] = "feature_fusion/Conv_7/Sigmoid";
	m_vLayerName[1] = "feature_fusion/concat_3";

	return true;
}

bool _DNNtext::start(void)
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

void _DNNtext::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		IF_CONT(!detect());

		m_obj.update();

		if (m_bGoSleep)
		{
			m_obj.m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _DNNtext::check(void)
{
	NULL__(m_pVision, -1);
	Frame* pBGR = m_pVision->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);
	IF__(pBGR->tStamp() <= m_BGR.tStamp(), -1);

	return 0;
}

bool _DNNtext::detect(void)
{
	IF_F(check() < 0);

	Frame* pBGR = m_pVision->BGR();
	m_BGR.copy(*pBGR);
	Mat mIn = *m_BGR.m();

	vInt4 iRoi;
	iRoi.x = mIn.cols * m_roi.x;
	iRoi.y = mIn.rows * m_roi.y;
	iRoi.z = mIn.cols * m_roi.z;
	iRoi.w = mIn.rows * m_roi.w;
	Rect rRoi;
	vInt42rect(iRoi, rRoi);
	Mat mBGR = mIn(rRoi);

	m_blob = blobFromImage(mBGR, m_scale, Size(m_nW, m_nH),
			Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);
	m_net.setInput(m_blob);

	vector<Mat> vO;
#if CV_VERSION_MAJOR > 3
	m_net.forward(vO, m_vLayerName);
#endif

	Mat vScores = vO[0];
	Mat vGeometry = vO[1];

	std::vector<RotatedRect> vBoxes;
	std::vector<float> vConfidences;
	decode(vScores, vGeometry, m_thr, vBoxes, vConfidences);

	std::vector<int> vIndices;
	NMSBoxes(vBoxes, vConfidences, m_thr, m_nms, vIndices);

	float bX = 1.0 / (float) mIn.cols;
	float bY = 1.0 / (float) mIn.rows;
	float rX = (float) mIn.cols / (float) m_nW;
	float rY = (float) mIn.rows / (float) m_nH;

	for (size_t i = 0; i < vIndices.size(); i++)
	{
		OBJECT o;
		o.init();
		o.m_tStamp = m_tStamp;
		o.setTopClass(0, 1.0);

		Point2f pV[4];
		RotatedRect& box = vBoxes[vIndices[i]];
		box.points(pV);
		for(int p=0; p<4; p++)
		{
			o.m_pBB[p].x = (pV[p].x * rX + rRoi.x) * bX;
			o.m_pBB[p].y = (pV[p].y * rY + rRoi.y) * bY;
		}

		this->add(&o);
	}

	return true;
}

void _DNNtext::decode(const Mat& mScores, const Mat& mGeometry,
		float scoreThresh, std::vector<RotatedRect>& vDetections,
		std::vector<float>& vConfidences)
{
	IF_(mScores.dims != 4);
	IF_(mGeometry.dims != 4);
	IF_(mScores.size[0] != 1);
	IF_(mGeometry.size[0] != 1);
	IF_(mScores.size[1] != 1);
	IF_(mGeometry.size[1] != 5);
	IF_(mScores.size[2] != mGeometry.size[2]);
	IF_(mScores.size[3] != mGeometry.size[3]);
	vDetections.clear();

	const int height = mScores.size[2];
	const int width = mScores.size[3];

	for (int y = 0; y < height; y++)
	{
		const float* scoresData = mScores.ptr<float>(0, 0, y);
		const float* x0_data = mGeometry.ptr<float>(0, 0, y);
		const float* x1_data = mGeometry.ptr<float>(0, 1, y);
		const float* x2_data = mGeometry.ptr<float>(0, 2, y);
		const float* x3_data = mGeometry.ptr<float>(0, 3, y);
		const float* anglesData = mGeometry.ptr<float>(0, 4, y);
		for (int x = 0; x < width; ++x)
		{
			float score = scoresData[x];
			if (score < scoreThresh)
				continue;

			// Decode a prediction.
			// Multiple by 4 because feature maps are 4 time less than input image.
			float offsetX = x * 4.0f, offsetY = y * 4.0f;
			float angle = anglesData[x];
			float cosA = std::cos(angle);
			float sinA = std::sin(angle);
			float h = x0_data[x] + x2_data[x];
			float w = x1_data[x] + x3_data[x];

			Point2f offset(offsetX + cosA * x1_data[x] + sinA * x2_data[x],
					offsetY - sinA * x1_data[x] + cosA * x2_data[x]);
			Point2f p1 = Point2f(-sinA * h, -cosA * h) + offset;
			Point2f p3 = Point2f(-cosA * w, sinA * w) + offset;
			RotatedRect r(0.5f * (p1 + p3), Size2f(w, h),
					-angle * 180.0f / (float) CV_PI);
			vDetections.push_back(r);
			vConfidences.push_back(score);
		}
	}
}

bool _DNNtext::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Scalar col = Scalar(0, 0, 255);

	OBJECT* pO;
	int i = 0;
	while ((pO = m_obj.at(i++)) != NULL)
	{
        line(*pMat, Point2f(pO->m_pBB[0].x*cs.x, pO->m_pBB[0].y*cs.y),
        			Point2f(pO->m_pBB[1].x*cs.x, pO->m_pBB[1].y*cs.y),
					col, 1);

        line(*pMat, Point2f(pO->m_pBB[1].x*cs.x, pO->m_pBB[1].y*cs.y),
        			Point2f(pO->m_pBB[2].x*cs.x, pO->m_pBB[2].y*cs.y),
					col, 1);

        line(*pMat, Point2f(pO->m_pBB[2].x*cs.x, pO->m_pBB[2].y*cs.y),
        			Point2f(pO->m_pBB[3].x*cs.x, pO->m_pBB[3].y*cs.y),
					col, 1);

        line(*pMat, Point2f(pO->m_pBB[3].x*cs.x, pO->m_pBB[3].y*cs.y),
        			Point2f(pO->m_pBB[0].x*cs.x, pO->m_pBB[0].y*cs.y),
					col, 1);
	}

	return true;
}

}
