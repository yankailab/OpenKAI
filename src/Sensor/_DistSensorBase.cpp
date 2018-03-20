/*
 * DistSensorBase.h
 *
 *  Created on: Apr 13, 2017
 *      Author: yankai
 */

#include "_DistSensorBase.h"

namespace kai
{

_DistSensorBase::_DistSensorBase()
{
	m_pDiv = NULL;
	m_nDiv = 0;
	m_fovH = 360;
	m_fovV = 0.1;
	m_dDeg = 0;
	m_dDegInv = 0;
	m_rMin = 0.0;
	m_rMax = DBL_MAX;
	m_hdg = 0.0;
	m_calibScale = 1.0;
	m_calibOffset = 0.0;
	m_showScale = 1.0;
	m_showDegOffset = 0.0;
	m_bReady = false;
}

_DistSensorBase::~_DistSensorBase()
{
	reset();
}

bool _DistSensorBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,fovH);
	IF_Fl(m_fovH <= 0, "angleTo > 0");
	IF_Fl(m_fovH > 360, "angleTo <= 360");
	KISSm(pK,fovV);

	KISSm(pK,nDiv);
	m_dDeg = m_fovH/m_nDiv;
	m_dDegInv = 1.0/m_dDeg;

	KISSm(pK,showScale);
	KISSm(pK,showDegOffset);
	KISSm(pK,rMin);
	KISSm(pK,rMax);
	KISSm(pK,calibScale);
	KISSm(pK,calibOffset);

	void* pKmed = pK->o("filterMed");
	void* pKavr = pK->o("filterAvr");

	IF_F(m_nDiv >= MAX_DIST_SENSOR_DIV);
	m_pDiv = new DIST_SENSOR_DIV[m_nDiv];
	for(int i=0;i<m_nDiv;i++)
	{
		m_pDiv[i].init(pKmed, pKavr);
	}

	return true;
}

bool _DistSensorBase::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	return true;
}

void _DistSensorBase::reset(void)
{
	this->_ThreadBase::reset();
	DEL(m_pDiv);
	m_nDiv = 0;
	m_bReady = false;
}

bool _DistSensorBase::bReady(void)
{
	return m_bReady;
}

void _DistSensorBase::update(void)
{
}

DIST_SENSOR_TYPE _DistSensorBase::type(void)
{
	return dsUnknown;
}

double _DistSensorBase::rMin(void)
{
	return m_rMin;
}

double _DistSensorBase::rMax(void)
{
	return m_rMax;
}

double _DistSensorBase::d(void)
{
	return -1.0;
}

double _DistSensorBase::d(vInt4* pROI, vInt2* pPos)
{
	return -1.0;
}

double _DistSensorBase::d(vDouble4* pROI, vInt2* pPos)
{
	return -1.0;
}

void _DistSensorBase::input(double deg, double d)
{
	IF_(!m_bReady);
	IF_(deg < 0);

	if(d < m_rMin)d = m_rMax;
	if(d > m_rMax)d = m_rMax;

	deg += m_hdg;
	int iDiv = (int) (deg * m_dDegInv);
	while (iDiv >= m_nDiv)
		iDiv -= m_nDiv;

	m_pDiv[iDiv].input(d);
}

void _DistSensorBase::input(double deg, double d, double a)
{
	IF_(!m_bReady);
	IF_(deg < 0);

	if(d < m_rMin)d = m_rMax;
	if(d > m_rMax)d = m_rMax;

	deg += m_hdg;
	int iDiv = (int) (deg * m_dDegInv);
	while (iDiv >= m_nDiv)
		iDiv -= m_nDiv;

	m_pDiv[iDiv].input(d,a);
}

double _DistSensorBase::d(double deg)
{
	if(!m_bReady)return -1.0;

	deg += m_hdg;
	while (deg >= DEG_AROUND)
		deg -= DEG_AROUND;

	int iDiv = (int) (deg * m_dDegInv);
	if(iDiv >= m_nDiv)iDiv = m_nDiv;
	Average* pD = &m_pDiv[iDiv].m_fAvr;

	double d = pD->v();
	if(d <= m_rMin)return -1.0;
	if(d > m_rMax)return -1.0;

	return d;
}

double _DistSensorBase::dMin(double degFrom, double degTo)
{
	IF_F(!m_bReady);

	degFrom += m_hdg;
	degTo += m_hdg;
	double degMid = 0.5 * (degFrom + degTo);

	int iFrom = (int) (degFrom * m_dDegInv);
	int iTo = (int) (degTo * m_dDegInv);

	double dist = m_rMax;
	int iMin = -1;

	for(int i=iFrom; i<iTo; i++)
	{
		int iDiv = i;
		while (iDiv >= m_nDiv)
			iDiv -= m_nDiv;

		Average* pD = &m_pDiv[iDiv].m_fAvr;

		double d = pD->v();
		IF_CONT(d <= m_rMin);
		IF_CONT(d > m_rMax * 0.99);

		d *= cos((i*m_dDeg - degMid)*DEG_RAD);
		IF_CONT(d >= dist);

		dist = d;
		iMin = iDiv;
	}

	if(iMin < 0)return -1.0;

	return dist;
}

double _DistSensorBase::dMax(double degFrom, double degTo)
{
	if(!m_bReady)return -1.0;

	degFrom += m_hdg;
	degTo += m_hdg;

	int iFrom = (int) (degFrom * m_dDegInv);
	int iTo = (int) (degTo * m_dDegInv);

	double dist = 0.0;
	int iMax = -1;

	for(int i=iFrom; i<iTo; i++)
	{
		int iDiv = i;
		while (iDiv >= m_nDiv)
			iDiv -= m_nDiv;

		Average* pD = &m_pDiv[iDiv].m_fAvr;

		double d = pD->v();
		IF_CONT(d <= m_rMin);
		IF_CONT(d > m_rMax);
		IF_CONT(d <= dist);

		dist = d;
		iMax = iDiv;
	}

	if(iMax < 0)return -1.0;
	return dist;
}

double _DistSensorBase::dAvr(double degFrom, double degTo)
{
	if(!m_bReady)return -1.0;

	degFrom += m_hdg;
	degTo += m_hdg;

	int iFrom = (int) (degFrom * m_dDegInv);
	int iTo = (int) (degTo * m_dDegInv);

	double dist = 0.0;
	int n = 0;

	for(int i=iFrom; i<iTo; i++)
	{
		int iDiv = i;
		while (iDiv >= m_nDiv)
			iDiv -= m_nDiv;

		Average* pD = &m_pDiv[iDiv].m_fAvr;

		double d = pD->v();
		IF_CONT(d <= m_rMin);
		IF_CONT(d > m_rMax);

		dist += d;
		n++;
	}

	if(n<=0)return -1.0;
	return dist/n;
}

bool _DistSensorBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	IF_F(!m_bReady);

	pWin->tabNext();
//	pWin->addMsg(&msg);
	pWin->tabPrev();

	IF_T(m_nDiv <= 0);

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	//Plot lidar result
	double rad = 0.0;
	double dRad = m_dDeg * DEG_RAD;

	for (int i = 0; i < m_nDiv; i++)
	{
		Average* pD = &m_pDiv[i].m_fAvr;

		double dist = pD->v();
		IF_CONT(dist <= m_rMin);
		IF_CONT(dist > m_rMax);
		dist *= m_showScale;

		rad += dRad;
		int pX = dist * sin(rad);
		int pY = -dist * cos(rad);

		Scalar col = Scalar(255, 255, 255);
		circle(*pMat, pCenter + Point(pX, pY), 1, col, 2);
	}

	return true;
}

}
