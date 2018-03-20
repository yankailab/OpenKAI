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

	void init(void* pKmed, void* pKavr)
	{
		m_fMed.init(pKmed);
		m_fAvr.init(pKavr);
		m_d = 0.0;
		m_a = 0.0;
	}

	void input(double d)
	{
		m_d = d;
		m_fMed.input(d);
		m_fAvr.input(m_fMed.v());
	}

	void input(double d, double a)
	{
		input(d);
		m_a = a;
	}

	double v(void)
	{
		return m_d;
	}

	double a(void)
	{
		return m_a;
	}
};


class _DistSensorBase: public _ThreadBase
{
public:
	_DistSensorBase();
	virtual ~_DistSensorBase();

	bool init(void* pKiss);
	bool link(void);
	bool draw(void);
	void reset(void);

	double rMin(void);
	double rMax(void);
	void input(double deg, double d);
	void input(double deg, double d, double a);
	void update(void);
	bool bReady(void);

	virtual DIST_SENSOR_TYPE type(void);
	virtual double d(void);
	virtual double d(vInt4* pROI, vInt2* pPos);
	virtual double d(vDouble4* pROI, vInt2* pPos);
	virtual double d(double deg);

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
