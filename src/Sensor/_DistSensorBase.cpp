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
	m_nDiv = 0;
	m_bReady = false;
	DEL_ARRAY(m_pDiv);
}

bool _DistSensorBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("fovH",&m_fovH);
	IF_Fl(m_fovH <= 0, "angleTo > 0");
	IF_Fl(m_fovH > 360, "angleTo <= 360");
	pK->v("fovV",&m_fovV);

	pK->v("nDiv",&m_nDiv);
	m_dDeg = m_fovH/m_nDiv;
	m_dDegInv = 1.0/m_dDeg;

	pK->v("showScale",&m_showScale);
	pK->v("showDegOffset",&m_showDegOffset);
	pK->v("rMin",&m_rMin);
	pK->v("rMax",&m_rMax);
	pK->v("calibScale",&m_calibScale);
	pK->v("calibOffset",&m_calibOffset);

	int nMed=0;
	int nAvr=0;
	pK->v("nMed", &nMed);
	pK->v("nAvr", &nAvr);

	IF_F(m_nDiv >= MAX_DIST_SENSOR_DIV);

	m_pDiv = new DIST_SENSOR_DIV[m_nDiv];
	for(int i=0;i<m_nDiv;i++)
	{
		m_pDiv[i].init(nAvr, nMed);
	}

	return true;
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
	return ds_Unknown;
}

float _DistSensorBase::rMin(void)
{
	return m_rMin;
}

float _DistSensorBase::rMax(void)
{
	return m_rMax;
}

void _DistSensorBase::input(float deg, float d, float a)
{
	IF_(!m_bReady);
	IF_(deg < 0);

	if(d < m_rMin)d = -1;
	if(d > m_rMax)d = -1;

	deg += m_hdg;
	int iDiv = (int) (deg * m_dDegInv);
	while (iDiv >= m_nDiv)
		iDiv -= m_nDiv;

	m_pDiv[iDiv].input(d,a);
}

float _DistSensorBase::d(int iDiv)
{
	if(!m_bReady)return -1.0;

	if(iDiv >= m_nDiv)iDiv = m_nDiv;

	return m_pDiv[iDiv].dAvr();
}

float _DistSensorBase::d(float deg)
{
	if(!m_bReady)return -1.0;

	deg += m_hdg;
	while (deg >= DEG_AROUND)
		deg -= DEG_AROUND;

	int iDiv = (int) (deg * m_dDegInv);
	if(iDiv >= m_nDiv)iDiv = m_nDiv;

	return m_pDiv[iDiv].dAvr();
}

float _DistSensorBase::dMin(void)
{
	if(!m_bReady)return -1.0;

	float degMid = 0.5 * m_fovH;
	float dist = m_rMax;
	int iMin = -1;

	for(int i=0; i<m_nDiv; i++)
	{
		float d = m_pDiv[i].dAvr();
		IF_CONT(d < 0.0);

		d *= cos((i*m_dDeg - degMid)*DEG2RAD);
		IF_CONT(d >= dist);

		dist = d;
		iMin = i;
	}

	if(iMin < 0)return -1.0;
	return dist;
}

float _DistSensorBase::dMax(void)
{
	if(!m_bReady)return -1.0;

	float degMid = 0.5 * m_fovH;
	float dist = 0.0;
	int iMax = -1;

	for(int i=0; i<m_nDiv; i++)
	{
		float d = m_pDiv[i].dAvr();
		IF_CONT(d < 0.0);

		d *= cos((i*m_dDeg - degMid)*DEG2RAD);
		IF_CONT(d <= dist);

		dist = d;
		iMax = i;
	}

	if(iMax < 0)return -1.0;
	return dist;
}

float _DistSensorBase::dAvr(void)
{
	if(!m_bReady)return -1.0;

	float degMid = 0.5 * m_fovH;
	float dist = 0.0;
	int n = 0;

	for(int i=0; i<m_nDiv; i++)
	{
		float d = m_pDiv[i].dAvr();
		IF_CONT(d < 0.0);

		d *= cos((i*m_dDeg - degMid)*DEG2RAD);
		dist += d;
		n++;
	}

	if(n<=0)return -1.0;
	return dist/n;
}

float _DistSensorBase::dMin(float degFrom, float degTo)
{
	if(!m_bReady)return -1.0;

	degFrom += m_hdg;
	degTo += m_hdg;
	float degMid = 0.5 * (degFrom + degTo);
	int iFrom = (int) (degFrom * m_dDegInv);
	int iTo = (int) (degTo * m_dDegInv);

	float dist = m_rMax;
	int iMin = -1;

	for(int i=iFrom; i<iTo; i++)
	{
		int iDiv = i;
		while (iDiv >= m_nDiv)
			iDiv -= m_nDiv;

		float d = m_pDiv[iDiv].dAvr();
		IF_CONT(d < 0.0);

		d *= cos((i*m_dDeg - degMid)*DEG2RAD);
		IF_CONT(d >= dist);

		dist = d;
		iMin = iDiv;
	}

	if(iMin < 0)return -1.0;
	return dist;
}

float _DistSensorBase::dMax(float degFrom, float degTo)
{
	if(!m_bReady)return -1.0;

	degFrom += m_hdg;
	degTo += m_hdg;
	float degMid = 0.5 * (degFrom + degTo);
	int iFrom = (int) (degFrom * m_dDegInv);
	int iTo = (int) (degTo * m_dDegInv);

	float dist = 0.0;
	int iMax = -1;

	for(int i=iFrom; i<iTo; i++)
	{
		int iDiv = i;
		while (iDiv >= m_nDiv)
			iDiv -= m_nDiv;

		float d = m_pDiv[iDiv].dAvr();
		IF_CONT(d < 0.0);

		d *= cos((i*m_dDeg - degMid)*DEG2RAD);
		IF_CONT(d <= dist);

		dist = d;
		iMax = iDiv;
	}

	if(iMax < 0)return -1.0;
	return dist;
}

float _DistSensorBase::dAvr(float degFrom, float degTo)
{
	if(!m_bReady)return -1.0;

	degFrom += m_hdg;
	degTo += m_hdg;

	int iFrom = (int) (degFrom * m_dDegInv);
	int iTo = (int) (degTo * m_dDegInv);

	float dist = 0.0;
	int n = 0;

	for(int i=iFrom; i<iTo; i++)
	{
		int iDiv = i;
		while (iDiv >= m_nDiv)
			iDiv -= m_nDiv;

		float d = m_pDiv[iDiv].dAvr();
		IF_CONT(d < 0.0);

		dist += d;
		n++;
	}

	if(n<=0)return -1.0;
	return dist/n;
}

void _DistSensorBase::draw(void)
{
	this->_ThreadBase::draw();

	string msg = "| ";
	for (int i = 0; i < m_nDiv; i++)
	{
		msg += f2str(m_pDiv[i].dAvr()) + " | ";
	}
	addMsg(msg);

	IF_(!checkWindow());
	IF_(!m_bReady);
	IF_(m_nDiv <= 0);
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	//Plot lidar result
	float rad = 0.0;
	float dRad = m_dDeg * DEG2RAD;

	for (int i = 0; i < m_nDiv; i++)
	{
		float dist = m_pDiv[i].dAvr();
		IF_CONT(dist <= m_rMin);
		IF_CONT(dist > m_rMax);
		dist *= m_showScale;

		rad += dRad;
		int pX = dist * sin(rad);
		int pY = -dist * cos(rad);

		Scalar col = Scalar(0, 255, 0);
		circle(*pMat, pCenter + Point(pX, pY), 1, col, 2);
	}

}

}
