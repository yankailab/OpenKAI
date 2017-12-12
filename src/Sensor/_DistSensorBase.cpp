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
	m_range.x = 0.0;
	m_range.y = DBL_MAX;
	m_offsetAngle = 0.0;
	m_showScale = 1.0;
	m_bReady = 0;
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
	F_INFO(pK->v("rangeFrom", &m_range.x));
	F_INFO(pK->v("rangeTo", &m_range.y));

	IF_F(m_nDiv >= MAX_DIST_SENSOR_DIV);
	m_pDiv = new DIST_SENSOR_DIV[m_nDiv];
	for(int i=0;i<m_nDiv;i++)
	{
		m_pDiv[i].init();
		//TODO: add filter window length
	}

	return true;
}

void _DistSensorBase::reset(void)
{
	this->_ThreadBase::reset();
	DEL(m_pDiv);
}

DIST_SENSOR_TYPE _DistSensorBase::type(void)
{
	return dsUnknown;
}

vDouble2 _DistSensorBase::range(void)
{
	return m_range;
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
/*
	//Plot lidar result
	for (int i = 0; i < m_nDiv; i++)
	{
		Average* pD = &m_pDistAvr[i];

		double dist = pD->v();
		IF_CONT(dist <= m_minDist);
		IF_CONT(dist > m_maxDist);

		dist *= m_showScale;
		double rad = m_dAngle * i * DEG_RAD;
		int pX = -dist * cos(rad);
		int pY = -dist * sin(rad);

		Scalar col = Scalar(255, 255, 255);
		circle(*pMat, pCenter + Point(pX, pY), 1, col, 2);
	}
*/
	return true;
}

}
