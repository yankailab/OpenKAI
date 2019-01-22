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
	m_thr = 0.5;
	m_max = 255;
}

_Thermal::~_Thermal()
{
}

bool _Thermal::init(void* pKiss)
{
	IF_F(!this->_ObjectBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,max);
	KISSm(pK,thr);
	m_thr *= m_max;

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

void _Thermal::detect(void)
{
	IF_(check()<0);

#ifdef USE_CUDA

	GpuMat gBGR = *(m_pVision->BGR()->gm());
	GpuMat gGray;
	cuda::cvtColor(gBGR, gGray, COLOR_BGR2GRAY);

	GpuMat gThr;
	cuda::threshold(gGray, gThr, m_thr, m_max, THRESH_BINARY); //THRESH_BINARY_INV);

	gThr.download(m_mThr);

#else

	Mat mBGR = *(m_pVision->BGR()->m());
	Mat mGray;
	cv::cvtColor(mBGR, mGray, COLOR_BGR2GRAY);

	cv::threshold(mGray, m_mThr, m_thr, m_max, THRESH_BINARY); //THRESH_BINARY_INV);

#endif

	vector< vector< Point > > vvContours;
	findContours(m_mThr, vvContours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	vInt2 cs;
	m_pVision->info(&cs, NULL, NULL);

	OBJECT o;
	vector<Point> vPoly;
	for (unsigned int i=0; i<vvContours.size(); i++)
	{
		vPoly.clear();
		approxPolyDP( vvContours[i], vPoly, 3, true );
		Rect rBB = boundingRect(vPoly);
		vInt4 iBB;
		rect2vInt4(rBB,iBB);

		o.init();
		o.m_tStamp = m_tStamp;
		o.setBB(iBB, cs);
		o.setTopClass(0, o.m_bb.area());

		add(&o);
		LOG_I("ID: "+ i2str(o.m_topClass));
	}
}

bool _Thermal::draw(void)
{
	IF_F(!this->_ObjectBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	if(!m_mThr.empty())
	{
		imshow(*this->getName()+":Thr", m_mThr);
	}

	return true;
}

}
