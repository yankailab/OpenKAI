/*
 * DistanceSensorBase.h
 *
 *  Created on: Apr 13, 2017
 *      Author: yankai
 */

#ifndef SRC_SENSOR_DISTANCESENSORBASE_H_
#define SRC_SENSOR_DISTANCESENSORBASE_H_

#include "../Base/common.h"

#define N_DIR_SENSOR 16

namespace kai
{

enum DISTANCE_SENSOR_TYPE
{
	dsUnknown,
	dsZED,
	dsSF40,
	dsLeddarVu,
};

struct DIR_DIST
{
	double m_rMin;
	double m_rMax;
	double m_dist;

	void init(void)
	{
		m_rMin = 0.0;
		m_rMax = DBL_MAX;
		m_dist = -1.0;
	}
};

class DistanceSensorBase
{
public:
	DistanceSensorBase();
	virtual ~DistanceSensorBase();

	virtual DISTANCE_SENSOR_TYPE type(void);
	virtual vDouble2 range(void);
	virtual uint8_t orientation(void);
	virtual double d(void);

public:
	uint8_t	m_orientation;
	DIR_DIST m_dd[N_DIR_SENSOR];
	vDouble2	m_range;


};

}

#endif
