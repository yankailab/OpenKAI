/*
 * _DistSensorBase.h
 *
 *  Created on: Apr 13, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__DistSensorBase_H_
#define OpenKAI_src_Sensor__DistSensorBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Filter/Median.h"
#include "../Filter/Average.h"
#include "../Protocol/_Mavlink.h"

#define MAX_DIST_SENSOR_DIV 720

namespace kai
{

enum DIST_SENSOR_TYPE
{
	dsUnknown,
	dsZED,
	dsSF40,
	dsLeddarVu,
	dsRPLIDAR
};

struct DIST_SENSOR_DIV
{
	double m_d;	//raw input
	double m_a; //amplitude
	Median m_fMed;
	Average m_fAvr;

	void init(int nAvr, int nMed)
	{
		m_fMed.init(nMed,0);
		m_fAvr.init(nAvr,0);
		m_d = -1.0;
		m_a = -1.0;
	}

	void input(double d, double a)
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

	double d(void)
	{
		return m_d;
	}

	double a(void)
	{
		return m_a;
	}

	double dAvr(void)
	{
		if(m_d < 0.0)return -1.0;
		return m_fAvr.v();
	}

	double dMed(void)
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
	bool draw(void);
	bool cli(int& iY);
	void update(void);

	double rMin(void);
	double rMax(void);
	void input(double deg, double d);
	void input(double deg, double d, double a);
	bool bReady(void);

	virtual DIST_SENSOR_TYPE type(void);
	virtual double d(double deg);
	virtual double dMin(void);
	virtual double dMax(void);
	virtual double dAvr(void);
	virtual double dMin(double degFrom, double degTo);
	virtual double dMax(double degFrom, double degTo);
	virtual double dAvr(double degFrom, double degTo);

public:
	DIST_SENSOR_DIV* m_pDiv;
	int		m_nDiv;
	double	m_fovH;
	double	m_fovV;
	double	m_dDeg;
	double	m_dDegInv;
	double	m_rMin;
	double	m_rMax;
	double	m_hdg;	//given by external sensor e.g. compass
	double  m_calibScale;
	double  m_calibOffset;
	double  m_showScale;
	double  m_showDegOffset;
	uint16_t m_bReady;
};

}
#endif
