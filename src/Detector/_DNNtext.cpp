/*
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */
#include "_DNNtext.h"

#ifdef USE_OPENCV

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
	m_bOCR = false;
	m_bDetect = true;
	m_bWarp = false;

#ifdef USE_OCR
	m_pOCR = NULL;
#endif
}

_DNNtext::~_DNNtext()
{
}

bool _DNNtext::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("thr", &m_thr);
	pK->v("nms", &m_nms);
	pK->v("nW", &m_nW);
	pK->v("nH", &m_nH);
	pK->v("iBackend", &m_iBackend);
	pK->v("iTarget", &m_iTarget);
	pK->v("bSwapRB", &m_bSwapRB);
	pK->v("scale", &m_scale);
	pK->v("bDetect", &m_bDetect);
	pK->v("bOCR", &m_bOCR);
	pK->v("bWarp", &m_bWarp);
	pK->v("iClassDraw", &m_iClassDraw);
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

	IF_T(!m_bOCR);

#ifdef USE_OCR
	string iName = "";
	F_INFO(pK->v("OCR", &iName));
	m_pOCR = (OCR*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pOCR, iName + " not found");
#endif

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

		if(check() >= 0)
		{
			if(m_bDetect)
				detect();

			ocr();

			if (m_bGoSleep)
				m_pU->m_pPrev->clear();
		}

		this->autoFPSto();
	}
}

int _DNNtext::check(void)
{
	NULL__(m_pU,-1);
	NULL__(m_pV, -1);
	Frame* pBGR = m_pV->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);
	IF__(pBGR->tStamp() <= m_fBGR.tStamp(), -1);

	return 0;
}

void _DNNtext::detect(void)
{
	m_fBGR.copy(*m_pV->BGR());
	if(m_fBGR.m()->channels()<3)
		m_fBGR.copy(m_fBGR.cvtColor(8));

	Mat mIn = *m_fBGR.m();
	m_blob = blobFromImage(mIn, m_scale, Size(m_nW, m_nH),
			Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);
	m_net.setInput(m_blob);

	vector<Mat> vO;
	m_net.forward(vO, m_vLayerName);

	Mat vScores = vO[0];
	Mat vGeometry = vO[1];

	std::vector<RotatedRect> vBoxes;
	std::vector<float> vConfidences;
	decode(vScores, vGeometry, m_thr, vBoxes, vConfidences);

	std::vector<int> vIndices;
	NMSBoxes(vBoxes, vConfidences, m_thr, m_nms, vIndices);

	vInt2 cs;
	cs.x = mIn.cols;
	cs.y = mIn.rows;

	float kx = (float)mIn.cols/(float)m_nW;
	float ky = (float)mIn.rows/(float)m_nH;

	for (size_t i = 0; i < vIndices.size(); i++)
	{
		_Object o;
		o.init();
		o.m_tStamp = m_tStamp;
		o.setTopClass(0, 1.0);

		Point2f pP[4];	//in pixel unit
		RotatedRect& box = vBoxes[vIndices[i]];
		box.points(pP);

		vFloat2 pV[4];
		for (int p = 0; p < 4; p++)
		{
			pV[p].x = pP[p].x * kx;
			pV[p].y = pP[p].y * ky;
		}
		o.setVertices2D(pV,4);
		o.normalize(mIn.cols, mIn.rows);

		m_pU->add(o);
	}

	m_pU->updateObj();
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

void _DNNtext::ocr(void)
{
#ifdef USE_OCR
	NULL_(m_pOCR);

	Mat mIn = *m_pV->BGR()->m();

	if(!m_bWarp)
	{
		m_pOCR->setMat(mIn);
	}

	_Object* pO;
	int i = 0;
	while ((pO = m_pU->get(i++)) != NULL)
	{
		Rect r = bb2Rect(pO->getBB2DNormalizedBy(mIn.cols, mIn.rows));
		Mat mRoi = mIn(r);
		Mat m;
		string strO;

		if(m_bWarp)
		{
			Mat mT = getTransform(mRoi, pO->m_pV);
			cv::warpPerspective(mRoi,
								m,
								mT,
								m.size(),
								cv::INTER_LINEAR);
			m_pOCR->setMat(m);
			strO = m_pOCR->scan(0);
		}
		else
		{
			m = mRoi;
			strO = m_pOCR->scan(&r);
		}

		int n = strO.length();
		if(n+1 > OBJ_N_CHAR)
			n = OBJ_N_CHAR-1;
		if(n>0)
			strncpy(pO->m_pText, strO.c_str(), n);
		pO->setText("");

		LOG_I(strO);
	}
#endif
}

Mat _DNNtext::getTransform(Mat mImg, vFloat2* pBox)
{
	Mat mP;
	if(!pBox)return mP;

	vFloat2 p;
	p = pBox[0];
	Point2f LT = Point2f((float) (p.x * mImg.cols),
						 (float) (p.y * mImg.rows));

	p = pBox[1];
	Point2f LB = Point2f((float) (p.x * mImg.cols),
						 (float) (p.y * mImg.rows));

	p = pBox[2];
	Point2f RB = Point2f((float) (p.x * mImg.cols),
						 (float) (p.y * mImg.rows));

	p = pBox[3];
	Point2f RT = Point2f((float) (p.x * mImg.cols),
						 (float) (p.y * mImg.rows));

	//LT, LB, RB, RT
	Point2f ptsFrom[] = { LT, LB, RB, RT };
	Point2f ptsTo[] =
	{ cv::Point2f(0, 0),
	  cv::Point2f(0, (float) mImg.rows),
	  cv::Point2f((float) mImg.cols, (float) mImg.rows),
	  cv::Point2f((float) mImg.cols, 0) };

	mP = getPerspectiveTransform(ptsFrom, ptsTo);
	return mP;
}

void _DNNtext::draw(void)
{
	this->_DetectorBase::draw();
	IF_(!checkWindow());

	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Scalar col = Scalar(0, 255, 0);
	int t = 1;

	_Object* pO;
	int i = 0;
	while ((pO = m_pU->get(i++)) != NULL)
	{
		vFloat2* pV0 = pO->getVertex(0);
		vFloat2* pV1 = pO->getVertex(1);
		vFloat2* pV2 = pO->getVertex(2);
		vFloat2* pV3 = pO->getVertex(3);

		line(*pMat, Point2f(pV0->x, pV0->y),
					Point2f(pV1->x, pV1->y), col, t);

		line(*pMat, Point2f(pV1->x, pV1->y),
					Point2f(pV2->x, pV2->y), col, t);

		line(*pMat, Point2f(pV2->x, pV2->y),
					Point2f(pV3->x, pV3->y), col, t);

		line(*pMat, Point2f(pV3->x, pV3->y),
					Point2f(pV0->x, pV0->y), col, t);

		Rect r = bb2Rect(pO->getBB2DNormalizedBy(pMat->cols, pMat->rows));
		rectangle(*pMat, r, col, t+1);

#ifdef USE_OCR
		IF_CONT(!m_bOCR);
		putText(*pMat, string(pO->getText()),
				Point(pO->getX() * pMat->cols,
					  pO->getY() * pMat->rows),
				FONT_HERSHEY_SIMPLEX, 1.5, col, 2);
#endif
	}
}

}
#endif
