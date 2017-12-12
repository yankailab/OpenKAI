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

	void init(void)
	{

	}
};


class _DistSensorBase: public _ThreadBase
{
public:
	_DistSensorBase();
	virtual ~_DistSensorBase();

	bool init(void* pKiss);
	bool draw(void);
	void reset(void);

	virtual DIST_SENSOR_TYPE type(void);
	virtual vDouble2 range(void);
	virtual double d(void);
	virtual double d(vInt4* pROI, vInt2* pPos);
	virtual double d(vDouble4* pROI, vInt2* pPos);
	virtual double d(int iDiv);

public:
	DIST_SENSOR_DIV* m_pDiv;
	int		m_nDiv;
	double	m_dAngle;
	vDouble2 m_range;
	double	m_offsetAngle;
	double  m_showScale;
	uint16_t m_bReady;

};

}

#endif
