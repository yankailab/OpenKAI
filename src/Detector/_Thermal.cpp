/*
 * _Thermal.cpp
 *
 *  Created on: Jan 18, 2019
 *      Author: yankai
 */

#include "_Thermal.h"

namespace kai
{

_Thermal::_Thermal()
{
	m_abs = 90;
	m_scale1 = 0.25;
	m_scale2 = 0.0625;
	m_thr1 = 200;
	m_thr2 = 255;
}

_Thermal::~_Thermal()
{
}

bool _Thermal::init(void* pKiss)
{
	IF_F(!this->_ObjectBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,abs);
	KISSm(pK,scale1);
	KISSm(pK,scale2);
	KISSm(pK,thr1);
	KISSm(pK,thr2);

	m_nClass = 1;

	return true;
}

bool _Thermal::start(void)
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

void _Thermal::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();
		m_obj.update();

		if(m_bGoSleep)
		{
			m_obj.m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _Thermal::check(void)
{
	IF__(!m_pVision,-1);
	IF__(m_pVision->BGR()->bEmpty(),-1);

	return 0;
}

#ifdef USE_CUDA
void _Thermal::detect(void)
{
	IF_(check()<0);


/*
    vector<int> vHistLev = { m_nHistLev };
	vector<float> vRange = { (float)m_vRange.x, (float)m_vRange.y };
	vector<int> vChannel = { 0 };

	Rect roi;
	vInt42rect(*pROI, roi);
	Mat mRoi = (*m_fDepth.m())(roi);
	vector<Mat> vRoi = {mRoi};
	Mat mHist;
	cv::calcHist(vRoi, vChannel, Mat(),
	            mHist, vHistLev, vRange,
	            true	//accumulate
				);

	int nMinHist = m_minHistD * mRoi.cols * mRoi.rows;
	int i;
	for(i=0; i<m_nHistLev; i++)
	{
		if(mHist.at<float>(i) >= (float)nMinHist)break;
	}

	return m_vRange.x + (((double)i)/(double)m_nHistLev) * (m_vRange.y - m_vRange.x);






	GpuMat mBGR = *(m_pVision->BGR()->gm());
	GpuMat mHSV;
	cuda::cvtColor(mBGR, mHSV, COLOR_BGR2HSV);

	vector<GpuMat> vmHSV(3);
	split(mHSV, vmHSV);
	GpuMat mH = vmHSV[0];
	GpuMat mS = vmHSV[1];
	GpuMat mV = vmHSV[2];

	GpuMat  gHred;
	GpuMat  gScaleHred;
	GpuMat  gScaleS;
	GpuMat  gBulleye;
	GpuMat  gThr;
	cuda::absdiff(mH, Scalar(m_abs), gHred);
	cuda::multiply(gHred, Scalar(m_scale1), gScaleHred);
	cuda::multiply(mS, Scalar(m_scale2), gScaleS);
	cuda::multiply(gScaleHred, gScaleS, gBulleye);
	cuda::threshold(gBulleye, gThr, m_thr1, m_thr2, THRESH_BINARY); //THRESH_BINARY_INV);

	Mat mThr;
	gThr.download(mThr);
	vector< vector< Point > > vvContours;
	findContours(mThr, vvContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vInt2 cSize;
	m_pVision->info(&cSize, NULL, NULL);

	OBJECT obj;
	vector<Point> vPoly;
	for (unsigned int i=0; i<vvContours.size(); i++)
	{
		vPoly.clear();
		approxPolyDP( vvContours[i], vPoly, 3, true );
		Rect rBB = boundingRect(vPoly);
		vInt4 iBB;
		rect2vInt4(rBB,iBB);

		obj.init();
		obj.m_tStamp = m_tStamp;
		obj.setBB(iBB, cSize);
		obj.setTopClass(0, obj.m_bb.area());

		add(&obj);
		LOG_I("ID: "+ i2str(obj.m_topClass));
	}
	*/
}
#else
void _Thermal::detect(void)
{
	IF_(check()<0);
/*
	GpuMat mBGR = *(m_pVision->BGR()->gm());
	GpuMat mHSV;
	cuda::cvtColor(mBGR, mHSV, COLOR_BGR2HSV);

	vector<GpuMat> vmHSV(3);
	split(mHSV, vmHSV);
	GpuMat mH = vmHSV[0];
	GpuMat mS = vmHSV[1];
	GpuMat mV = vmHSV[2];

	GpuMat  gHred;
	GpuMat  gScaleHred;
	GpuMat  gScaleS;
	GpuMat  gBulleye;
	GpuMat  gThr;
	cuda::absdiff(mH, Scalar(m_abs), gHred);
	cuda::multiply(gHred, Scalar(m_scale1), gScaleHred);
	cuda::multiply(mS, Scalar(m_scale2), gScaleS);
	cuda::multiply(gScaleHred, gScaleS, gBulleye);
	cuda::threshold(gBulleye, gThr, m_thr1, m_thr2, THRESH_BINARY); //THRESH_BINARY_INV);

	Mat mThr;
	gThr.download(mThr);
	vector< vector< Point > > vvContours;
	findContours(mThr, vvContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vInt2 cSize;
	m_pVision->info(&cSize, NULL, NULL);

	OBJECT obj;
	vector<Point> vPoly;
	for (unsigned int i=0; i<vvContours.size(); i++)
	{
		vPoly.clear();
		approxPolyDP( vvContours[i], vPoly, 3, true );
		Rect rBB = boundingRect(vPoly);
		vInt4 iBB;
		rect2vInt4(rBB,iBB);

		obj.init();
		obj.m_tStamp = m_tStamp;
		obj.setBB(iBB, cSize);
		obj.setTopClass(0, obj.m_bb.area());

		add(&obj);
		LOG_I("ID: "+ i2str(obj.m_topClass));
	}
	*/
}
#endif

}