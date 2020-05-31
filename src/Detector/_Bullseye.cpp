/*
 * _Bullseye.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_Bullseye.h"

#ifdef USE_OPENCV
#ifdef USE_CUDA

namespace kai
{

_Bullseye::_Bullseye()
{
	m_abs = 90;
	m_scale1 = 0.25;
	m_scale2 = 0.0625;
	m_thr1 = 200;
	m_thr2 = 255;
}

_Bullseye::~_Bullseye()
{
}

bool _Bullseye::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("abs", &m_abs);
	pK->v("scale1", &m_scale1);
	pK->v("scale2", &m_scale2);
	pK->v("thr1", &m_thr1);
	pK->v("thr2", &m_thr2);

	m_nClass = 1;

	return true;
}

bool _Bullseye::start(void)
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

void _Bullseye::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		IF_CONT(check()<0);

		detect();
		m_pU->updateObj();

		if(m_bGoSleep)
		{
			m_pU->m_pPrev->clear();
		}

		this->autoFPSto();
	}
}

int _Bullseye::check(void)
{
	NULL__(m_pU,-1);
	NULL__(m_pV,-1);
	IF__(m_pV->BGR()->bEmpty(),-1);

	return 0;
}

void _Bullseye::detect(void)
{
	GpuMat mBGR = *(m_pV->BGR()->gm());
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
	findContours(mThr, vvContours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	float bW = 1.0/mBGR.cols;
	float bH = 1.0/mBGR.rows;

	_Object o;
	vector<Point> vPoly;
	for (unsigned int i=0; i<vvContours.size(); i++)
	{
		vPoly.clear();
		approxPolyDP( vvContours[i], vPoly, 3, true );
		Rect r = boundingRect(vPoly);

		o.init();
		o.m_tStamp = m_tStamp;
		o.setBB2D(rect2BB<vFloat4>(r));
		o.normalize(bW,bH);
		o.setTopClass(0, o.area());

		m_pU->add(o);
		LOG_I("ID: "+ i2str(o.m_topClass));
	}
}

}
#endif
#endif
