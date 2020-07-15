/*
 * _DistSensorBase.h
 *
 *  Created on: Apr 13, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__DistSensorBase_H_
#define OpenKAI_src_Sensor__DistSensorBase_H_

#include "../Base/_ThreadBase.h"
#include "../Filter/Median.h"
#include "../Filter/Average.h"

#define MAX_DIST_SENSOR_DIV 720

namespace kai
{

enum DIST_SENSOR_TYPE
{
	ds_Unknown,
	ds_RPLIDAR,
	ds_LeddarVu,
	ds_TOFsense,
};

struct DIST_SENSOR_DIV
{
	float m_d;	//raw input
	float m_a; //amplitude
	Median m_fMed;
	Average m_fAvr;

	void init(int nAvr, int nMed)
	{
		m_fMed.init(nMed,0);
		m_fAvr.init(nAvr,0);
		m_d = -1.0;
		m_a = -1.0;
	}

	void input(float d, float a)
	{
		m_a = a;
		m_d = d;
		if(d >= 0.0)
		{
			m_fMed.input(d);
			m_fAvr.input(m_fMed.v());
			return;
		}

		reset();
	}

	float d(void)
	{
		return m_d;
	}

	float a(void)
	{
		return m_a;
	}

	float dAvr(void)
	{
		if(m_d < 0.0)return -1.0;
		return m_fAvr.v();
	}

	float dMed(void)
	{
		if(m_d < 0.0)return -1.0;
		return m_fMed.v();
	}

	void reset(void)
	{
		m_fMed.reset();
		m_fAvr.reset();
	}
};

class _DistSensorBase: public _ThreadBase
{
public:
	_DistSensorBase();
	virtual ~_DistSensorBase();

	bool init(void* pKiss);
	void draw(void);
	void update(void);

	float rMin(void);
	float rMax(void);
	void input(float deg, float d, float a = -1.0);
	void input(int iDiv, float d, float a = -1.0);
	bool bReady(void);

	virtual DIST_SENSOR_TYPE type(void);
	virtual float d(int iDiv);
	virtual float d(float deg);
	virtual float dMin(void);
	virtual float dMax(void);
	virtual float dAvr(void);
	virtual float dMin(float degFrom, float degTo);
	virtual float dMax(float degFrom, float degTo);
	virtual float dAvr(float degFrom, float degTo);

public:
	DIST_SENSOR_DIV* m_pDiv;
	int		m_nDiv;
	float	m_fovH;
	float	m_fovV;
	float	m_dDeg;
	float	m_dDegInv;
	float	m_rMin;
	float	m_rMax;
	float	m_hdg;	//given by external sensor e.g. compass
	float  m_calibScale;
	float  m_calibOffset;
	float  m_showScale;
	float  m_showDegOffset;
	uint16_t m_bReady;
};

}
#endif
