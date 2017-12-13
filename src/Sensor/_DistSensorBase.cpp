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
	m_dAngle = 0;
	m_rMin = 0.0;
	m_rMax = DBL_MAX;
	m_offsetAngle = 0.0;
	m_hdg = 0.0;
	m_showScale = 1.0;
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

	KISSm(pK,nDiv);
	m_dAngle = 360/m_nDiv;
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

void _DistSensorBase::reset(void)
{
	this->_ThreadBase::reset();
	DEL(m_pDiv);
	m_nDiv = 0;
}

void _DistSensorBase::input(double angle, double d)
{
	IF_(d <= m_rMin);
	IF_(d > m_rMax);
	IF_(angle < 0);

	angle += m_hdg + m_offsetAngle;
	while (angle >= DEG_AROUND)
		angle -= DEG_AROUND;

	int iAngle = (int) (angle / m_dAngle);
	IF_(iAngle >= m_nDiv);

	m_pDiv[iAngle].input(d);
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

double _DistSensorBase::d(int iDiv)
{
	return -1.0;
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
	for (int i = 0; i < m_nDiv; i++)
	{
		Average* pD = &m_pDiv[i].m_fAvr;

		double dist = pD->v();
		IF_CONT(dist <= m_rMin);
		IF_CONT(dist > m_rMax);

		dist *= m_showScale;
		double rad = m_dAngle * i * DEG_RAD;
		int pX = -dist * cos(rad);
		int pY = -dist * sin(rad);

		Scalar col = Scalar(255, 255, 255);
		circle(*pMat, pCenter + Point(pX, pY), 1, col, 2);
	}

	return true;
}

}
