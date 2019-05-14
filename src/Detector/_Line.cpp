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
	m_wSlide = 0.01;
	m_minPixLine = 0.005;
	m_line = -1.0;
}

_Line::~_Line()
{
}

bool _Line::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,minPixLine);
	KISSm(pK,wSlide);

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
	float nP = m_mIn.rows*m_mIn.cols;

	Mat vSum;
	cv::reduce(m_mIn, vSum, 1, cv::REDUCE_SUM, CV_32SC1);
	vSum *= (float)1.0/255.0;

	//sliding window
	int i,j;
	int iFrom = -1;
	int iTo = -1;
	int nSlide = m_wSlide * m_mIn.rows * 0.5;

	for(i=0; i<vSum.rows; i++)
	{
		int sFrom = constrain(i-nSlide, 0, vSum.rows);
		int sTo = constrain(i+nSlide, 0, vSum.rows);

		int v = 0;
		for(j=sFrom; j<sTo; j++)
			v += vSum.at<int>(j,0);

		if((float)v/nP > m_minPixLine)
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

	if(iFrom < 0)
	{
		m_line = -1.0;
		return;
	}

	iTo = constrain(iTo, iFrom+1, vSum.rows);

	vInt2 cs;
	cs.x = m_mIn.cols;
	cs.y = m_mIn.rows;

	Rect rBB;
	rBB.x = 0;
	rBB.y = iFrom;
	rBB.width = m_mIn.cols;
	rBB.height = iTo - iFrom;

	OBJECT o;
	o.init();
	o.m_tStamp = m_tStamp;
	o.setBB(rBB,cs);
	o.setTopClass(0, 1.0);
	add(&o);

	m_line = o.m_bb.midY();
	LOG_I("Line pos: " + f2str(m_line));
}

bool _Line::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	pWin->tabNext();
	pWin->addMsg("line = " + f2str(m_line));
	pWin->tabPrev();

	return true;
}

bool _Line::console(int& iY)
{
	IF_F(!this->_DetectorBase::console(iY));

	string msg;
	C_MSG("line = " + f2str(m_line));

	return true;
}

}
