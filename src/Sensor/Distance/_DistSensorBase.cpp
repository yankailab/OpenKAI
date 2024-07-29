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
		m_nDiv = 1;
		m_fovH = 360;
		m_fovV = 0.1;
		m_dDeg = 0;
		m_dDegInv = 0;
		m_vRange.set(0, FLT_MAX);
		m_calibScale = 1.0;
		m_calibOffset = 0.0;
		m_bReady = false;
	}

	_DistSensorBase::~_DistSensorBase()
	{
		m_nDiv = 0;
		m_bReady = false;
		DEL_ARRAY(m_pDiv);
	}

	bool _DistSensorBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("fovH", &m_fovH);
		IF_Fl(m_fovH <= 0, "angleTo > 0");
		IF_Fl(m_fovH > 360, "angleTo <= 360");
		pK->v("fovV", &m_fovV);

		pK->v("nDiv", &m_nDiv);
		m_dDeg = m_fovH / m_nDiv;
		m_dDegInv = 1.0 / m_dDeg;

		pK->v("vRange", &m_vRange);
		pK->v("calibScale", &m_calibScale);
		pK->v("calibOffset", &m_calibOffset);

		int nMed = 0;
		int nAvr = 0;
		pK->v("nMed", &nMed);
		pK->v("nAvr", &nAvr);

		IF_F(m_nDiv >= MAX_DIST_SENSOR_DIV);

		m_pDiv = new DIST_SENSOR_DIV[m_nDiv];
		for (int i = 0; i < m_nDiv; i++)
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
		return m_vRange.x;
	}

	float _DistSensorBase::rMax(void)
	{
		return m_vRange.y;
	}

	void _DistSensorBase::input(float deg, float d, float a)
	{
		IF_(!m_bReady);
		IF_(deg < 0);

		int iDiv = (int)(deg * m_dDegInv);
		while (iDiv >= m_nDiv)
			iDiv -= m_nDiv;

		input(iDiv, d, a);
	}

	void _DistSensorBase::input(int iDiv, float d, float a)
	{
		IF_(!m_bReady);
		IF_(iDiv < 0 || iDiv >= m_nDiv);

		if (!m_vRange.bInside(d))
			d = -1;

		m_pDiv[iDiv].input(d, a);
	}

	float _DistSensorBase::d(int iDiv)
	{
		if (!m_bReady)
			return -1.0;

		if (iDiv >= m_nDiv)
			iDiv = m_nDiv;

		return m_pDiv[iDiv].dAvr();
	}

	float _DistSensorBase::d(float deg)
	{
		if (!m_bReady)
			return -1.0;

		while (deg >= DEG_AROUND)
			deg -= DEG_AROUND;

		int iDiv = (int)(deg * m_dDegInv);
		if (iDiv >= m_nDiv)
			iDiv = m_nDiv;

		return m_pDiv[iDiv].dAvr();
	}

	float _DistSensorBase::dMin(void)
	{
		if (!m_bReady)
			return -1.0;

		float degMid = 0.5 * m_fovH;
		float dist = m_vRange.y;
		int iMin = -1;

		for (int i = 0; i < m_nDiv; i++)
		{
			float d = m_pDiv[i].dAvr();
			IF_CONT(d < 0.0);

			d *= cos((i * m_dDeg - degMid) * DEG_2_RAD);
			IF_CONT(d >= dist);

			dist = d;
			iMin = i;
		}

		if (iMin < 0)
			return -1.0;
		return dist;
	}

	float _DistSensorBase::dMax(void)
	{
		if (!m_bReady)
			return -1.0;

		float degMid = 0.5 * m_fovH;
		float dist = 0.0;
		int iMax = -1;

		for (int i = 0; i < m_nDiv; i++)
		{
			float d = m_pDiv[i].dAvr();
			IF_CONT(d < 0.0);

			d *= cos((i * m_dDeg - degMid) * DEG_2_RAD);
			IF_CONT(d <= dist);

			dist = d;
			iMax = i;
		}

		if (iMax < 0)
			return -1.0;
		return dist;
	}

	float _DistSensorBase::dAvr(void)
	{
		if (!m_bReady)
			return -1.0;

		float degMid = 0.5 * m_fovH;
		float dist = 0.0;
		int n = 0;

		for (int i = 0; i < m_nDiv; i++)
		{
			float d = m_pDiv[i].dAvr();
			IF_CONT(d < 0.0);

			d *= cos((i * m_dDeg - degMid) * DEG_2_RAD);
			dist += d;
			n++;
		}

		if (n <= 0)
			return -1.0;
		return dist / n;
	}

	float _DistSensorBase::dMin(float degFrom, float degTo)
	{
		if (!m_bReady)
			return -1.0;

		float degMid = 0.5 * (degFrom + degTo);
		int iFrom = (int)(degFrom * m_dDegInv);
		int iTo = (int)(degTo * m_dDegInv);

		float dist = m_vRange.y;
		int iMin = -1;

		for (int i = iFrom; i < iTo; i++)
		{
			int iDiv = i;
			while (iDiv >= m_nDiv)
				iDiv -= m_nDiv;

			float d = m_pDiv[iDiv].dAvr();
			IF_CONT(d < 0.0);

			d *= cos((i * m_dDeg - degMid) * DEG_2_RAD);
			IF_CONT(d >= dist);

			dist = d;
			iMin = iDiv;
		}

		if (iMin < 0)
			return -1.0;
		return dist;
	}

	float _DistSensorBase::dMax(float degFrom, float degTo)
	{
		if (!m_bReady)
			return -1.0;

		float degMid = 0.5 * (degFrom + degTo);
		int iFrom = (int)(degFrom * m_dDegInv);
		int iTo = (int)(degTo * m_dDegInv);

		float dist = 0.0;
		int iMax = -1;

		for (int i = iFrom; i < iTo; i++)
		{
			int iDiv = i;
			while (iDiv >= m_nDiv)
				iDiv -= m_nDiv;

			float d = m_pDiv[iDiv].dAvr();
			IF_CONT(d < 0.0);

			d *= cos((i * m_dDeg - degMid) * DEG_2_RAD);
			IF_CONT(d <= dist);

			dist = d;
			iMax = iDiv;
		}

		if (iMax < 0)
			return -1.0;
		return dist;
	}

	float _DistSensorBase::dAvr(float degFrom, float degTo)
	{
		if (!m_bReady)
			return -1.0;

		int iFrom = (int)(degFrom * m_dDegInv);
		int iTo = (int)(degTo * m_dDegInv);

		float dist = 0.0;
		int n = 0;

		for (int i = iFrom; i < iTo; i++)
		{
			int iDiv = i;
			while (iDiv >= m_nDiv)
				iDiv -= m_nDiv;

			float d = m_pDiv[iDiv].dAvr();
			IF_CONT(d < 0.0);

			dist += d;
			n++;
		}

		if (n <= 0)
			return -1.0;
		return dist / n;
	}

	void _DistSensorBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		string msg = "| ";
		for (int i = 0; i < m_nDiv; i++)
		{
			msg += f2str(m_pDiv[i].dAvr()) + " | ";
		}
		((_Console *)pConsole)->addMsg(msg);
	}

	void _DistSensorBase::draw(void* pFrame)
	{
#ifdef USE_OPENCV
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() != OK_OK);

		Frame *pF = (Frame*)pFrame;

		Mat *pM = pF->m();
		IF_(pM->empty());

		IF_(!m_bReady);
		IF_(m_nDiv <= 0);

		//Plot center as vehicle position
		Point pCenter(pM->cols / 2, pM->rows / 2);
		circle(*pM, pCenter, 10, Scalar(0, 0, 255), 2);

		//Plot lidar result
		float rad = 0.0;
		float dRad = m_dDeg * DEG_2_RAD;

		for (int i = 0; i < m_nDiv; i++)
		{
			float dist = m_pDiv[i].dAvr();
			IF_CONT(!m_vRange.bInside(dist));
//			dist *= m_showScale;

			rad += dRad;
			int pX = dist * sin(rad);
			int pY = -dist * cos(rad);

			Scalar col = Scalar(0, 255, 0);
			circle(*pM, pCenter + Point(pX, pY), 1, col, 2);
		}
#endif
	}

}
