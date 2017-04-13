/*
 * _DistanceSensorBase.h
 *
 *  Created on: Apr 13, 2017
 *      Author: yankai
 */

#include "DistanceSensorBase.h"

namespace kai
{

DistanceSensorBase::DistanceSensorBase()
{
	m_orientation = 0;
	m_range.x = 0.0;
	m_range.y = DBL_MAX;

}

DistanceSensorBase::~DistanceSensorBase()
{
}

uint8_t DistanceSensorBase::orientation(void)
{
	return 0;
}

DISTANCE_SENSOR_TYPE DistanceSensorBase::type(void)
{
	return dsUnknown;
}

vDouble2 DistanceSensorBase::range(void)
{
	return m_range;
}

double DistanceSensorBase::d(void)
{
	return -1.0;
}


}
