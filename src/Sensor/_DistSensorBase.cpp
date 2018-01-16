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
	m_fov = 360;
	m_dAngle = 0;
	m_dAngleInv = 0;
	m_rMin = 0.0;
	m_rMax = DBL_MAX;
	m_offsetAngle = 0.0;
	m_hdg = 0.0;
	m_showScale = 1.0;
	m_bReady = false;
	m_pMavlink = NULL;
	m_dT.init();
	m_diffMax = 1.0;
	m_diffMin = 0.0;
	m_odoConfidence = 0.0;
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

	KISSm(pK,fov);
	IF_Fl(m_fov <= 0, "angleTo > 0");
	IF_Fl(m_fov > 360, "angleTo <= 360");

	KISSm(pK,nDiv);
	m_dAngle = m_fov/m_nDiv;
	m_dAngleInv = 1.0/m_dAngle;

	KISSm(pK,offsetAngle);
	KISSm(pK,showScale);
	KISSm(pK,rMin);
	KISSm(pK,rMax);

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

	string iName = "";
	F_INFO(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void _DistSensorBase::reset(void)
{
	this->_ThreadBase::reset();
	DEL(m_pDiv);
	m_nDiv = 0;
}

void _DistSensorBase::update(void)
{
	if(m_pMavlink)
	{
		m_hdg = ((double)(m_pMavlink->m_msg.global_position_int.hdg))*0.01;
	}
}

void _DistSensorBase::updateOdometry(void)
{
	IF_(m_bReady);
	NULL_(m_pMavlink);

	int i;
	double pX = 0.0;
	double pY = 0.0;
	double nV = 0.0;
	double rad = 0.0;
	double dRad = m_dAngle * DEG_RAD;

	for (i = 0; i < m_nDiv; i++)
	{
		Average* pA = &m_pDiv[i].m_fAvr;

		double dist = pA->v();
		IF_CONT(dist < m_rMin);
		IF_CONT(dist > m_rMax);

		double dD = pA->accumlatedDiff();
		double absDD = abs(dD);
		IF_CONT(absDD <= m_diffMin);
		IF_CONT(absDD > m_diffMax);

		rad += dRad;
		pX += dD * cos(rad);
		pY += -dD * sin(rad);
		nV += 1.0;
	}

	m_odoConfidence = nV/m_nDiv;
	nV = 1.0/nV;
	m_dT.x = pX * nV;
	m_dT.y = pY * nV;
}

vDouble2 _DistSensorBase::dT(void)
{
	return m_dT;
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

void _DistSensorBase::input(double angle, double d)
{
	IF_(d <= m_rMin);
	IF_(d > m_rMax);
	IF_(angle < 0);
	IF_(angle > m_fov);

	angle += m_hdg + m_offsetAngle;
	while (angle >= DEG_AROUND)
		angle -= DEG_AROUND;

	int iAngle = (int) (angle * m_dAngleInv);
	if(iAngle >= m_nDiv)iAngle = m_nDiv;

	m_pDiv[iAngle].input(d);
}

double _DistSensorBase::d(double deg)
{
	deg += m_hdg + m_offsetAngle;
	while (deg >= DEG_AROUND)
		deg -= DEG_AROUND;

	int iAngle = (int) (deg * m_dAngleInv);
	if(iAngle >= m_nDiv)iAngle = m_nDiv;
	Average* pD = &m_pDiv[iAngle].m_fAvr;

	double d = pD->v();
	if(d <= m_rMin)return -1.0;
	if(d > m_rMax)return -1.0;

	return d;
}

double _DistSensorBase::dMin(double degFrom, double degTo)
{
	degFrom += m_hdg + m_offsetAngle;
	degTo += m_hdg + m_offsetAngle;

	int iFrom = (int) (degFrom * m_dAngleInv);
	int iTo = (int) (degTo * m_dAngleInv);

	double dist = m_rMax;
	int iMin = -1;

	for(int i=iFrom; i<iTo; i++)
	{
		int iDiv = i;
		while (iDiv >= DEG_AROUND)
			iDiv -= DEG_AROUND;

		Average* pD = &m_pDiv[iDiv].m_fAvr;

		double d = pD->v();
		IF_CONT(d <= m_rMin);
		IF_CONT(d > m_rMax);
		IF_CONT(d >= dist);

		dist = d;
		iMin = iDiv;
	}

	if(iMin < 0)return -1.0;
	return dist;
}

double _DistSensorBase::dMax(double degFrom, double degTo)
{
	degFrom += m_hdg + m_offsetAngle;
	degTo += m_hdg + m_offsetAngle;

	int iFrom = (int) (degFrom * m_dAngleInv);
	int iTo = (int) (degTo * m_dAngleInv);

	double dist = 0.0;
	int iMax = -1;

	for(int i=iFrom; i<iTo; i++)
	{
		int iDiv = i;
		while (iDiv >= DEG_AROUND)
			iDiv -= DEG_AROUND;

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
	degFrom += m_hdg + m_offsetAngle;
	degTo += m_hdg + m_offsetAngle;

	int iFrom = (int) (degFrom * m_dAngleInv);
	int iTo = (int) (degTo * m_dAngleInv);

	double dist = 0.0;
	int n = 0;

	for(int i=iFrom; i<iTo; i++)
	{
		int iDiv = i;
		while (iDiv >= DEG_AROUND)
			iDiv -= DEG_AROUND;

		Average* pD = &m_pDiv[iDiv].m_fAvr;

		double d = pD->v();
		IF_CONT(d <= m_rMin);
		IF_CONT(d > m_rMax);

		dist += d;
		n++;
	}

	if(n<=0)return -1;
	return dist;
}

bool _DistSensorBase::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	pWin->tabNext();
//	pWin->addMsg(&msg);
	pWin->tabPrev();

	IF_T(m_nDiv <= 0);

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	//Plot lidar result
	double rad = 0.0;
	double dRad = m_dAngle * DEG_RAD;

	for (int i = 0; i < m_nDiv; i++)
	{
		Average* pD = &m_pDiv[i].m_fAvr;

		double dist = pD->v();
		IF_CONT(dist <= m_rMin);
		IF_CONT(dist > m_rMax);
		dist *= m_showScale;

		rad += dRad;
		int pX = -dist * cos(rad);
		int pY = -dist * sin(rad);

		Scalar col = Scalar(255, 255, 255);
		circle(*pMat, pCenter + Point(pX, pY), 1, col, 2);
	}

	return true;
}

}
