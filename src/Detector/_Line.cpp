/*
 * _Line.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_Line.h"

namespace kai
{

_Line::_Line()
{
	m_pV = NULL;
	m_rDim = 0;
	m_wSlide = 10;
	m_vThr = 10;
	m_vTower = 150;
	m_bTower = false;
}

_Line::~_Line()
{
}

bool _Line::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,vTower);
	KISSm(pK,rDim);
	KISSm(pK,wSlide);
	m_wSlide /= 2;

	m_nClass = 1;

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _Line::start(void)
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

void _Line::update(void)
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

int _Line::check(void)
{
	IF__(!m_pV,-1);
	IF__(m_pV->BGR()->bEmpty(),-1);

	return 0;
}

void _Line::detect(void)
{
	IF_(check()<0);

	m_pV->BGR()->m()->copyTo(m_mIn);

	if((cv::sum(m_mIn)[0]/(m_mIn.rows*m_mIn.cols)) > m_vTower)
	{
		m_bTower = true;
	}
	else
	{
		m_bTower = false;
	}

	Mat vSum;
	cv::reduce(m_mIn, vSum, m_rDim, CV_REDUCE_SUM, CV_32SC1);

	//sliding window
	int i,j;
	int iFrom = -1;
	int iTo = -1;
	
	for(i=0; i<vSum.cols; i++)
	{
		int sFrom = constrain(i-m_wSlide, 0, vSum.cols);
		int sTo = constrain(i+m_wSlide, 0, vSum.cols);

		int v = 0;
		for(j=sFrom; j<sTo; j++)
			v += vSum.at<int>(0,j);

		if(v > m_vThr)
		{
			iFrom = i;
			break;
		}

//		if(v > m_vThr && iFrom < 0)
//		{
//			iFrom = i;
//		}
//		else if(v < m_vThr && iFrom >= 0)
//		{
//			iTo = i;
//			break;
//		}
	}

	IF_(iFrom < 0);
	iTo = constrain(iTo, iFrom+1, vSum.cols);

	vInt2 cs;
	cs.x = m_mIn.cols;
	cs.y = m_mIn.rows;

	Rect rBB;
	rBB.x = iFrom;
	rBB.y = 0;
	rBB.width = iTo - iFrom;
	rBB.height = m_mIn.rows;

	OBJECT o;
	o.init();
	o.m_tStamp = m_tStamp;
	o.setBB(rBB,cs);
	o.setTopClass(0, 1.0);
	add(&o);

	LOG_I("Line pos: " + f2str(o.m_bb.midX()));
}

bool _Line::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

//	if(!m_mIn.empty())
//	{
//		imshow(*this->getName(), m_mIn);
//	}

	return true;
}

bool _Line::console(int& iY)
{
	IF_F(!this->_DetectorBase::console(iY));

	string msg;
	C_MSG("bTower = " + i2str(m_bTower));

	return true;
}

}
