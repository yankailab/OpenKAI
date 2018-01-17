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
	Median m_fMed;
	Average m_fAvr;

	void init(void* pKmed, void* pKavr)
	{
		m_fMed.init(pKmed);
		m_fAvr.init(pKavr);
	}

	void input(double d)
	{
		m_fMed.input(d);
		m_fAvr.input(m_fMed.v());
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
	void update(void);
	void updateOdometry(void);
	vDouble2 dT(void);
	bool bReady(void);

	virtual DIST_SENSOR_TYPE type(void);
	virtual double d(void);
	virtual double d(vInt4* pROI, vInt2* pPos);
	virtual double d(vDouble4* pROI, vInt2* pPos);
	virtual double d(double deg);

	virtual double dMin(double degFrom, double degTo);
	virtual double dMax(double degFrom, double degTo);
	virtual double dAvr(double degFrom, double degTo);

	virtual bool dMin(double degFrom, double degTo, double* pDeg, double* pD);
	virtual bool dMax(double degFrom, double degTo, double* pDeg, double* pD);

public:
	DIST_SENSOR_DIV* m_pDiv;
	int		m_nDiv;
	double	m_fov;
	double	m_dDeg;
	double	m_dDegInv;
	double	m_rMin;
	double	m_rMax;
	double	m_offsetDeg;
	double	m_hdg;	//given by external sensor e.g. compass
	double  m_showScale;
	uint16_t m_bReady;

	//odometry
	_Mavlink* m_pMavlink;
	vDouble2 m_dT;	//dTranslation, x, y,
	double	m_diffMax;
	double	m_diffMin;
	double	m_odoConfidence;

};

}

#endif
