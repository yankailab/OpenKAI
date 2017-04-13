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
}

DistanceSensorBase::~DistanceSensorBase()
{
}

uint8_t DistanceSensorBase::getOrientation(void)
{
	return 0;//m_orientation;
}

DISTANCE_SENSOR_TYPE DistanceSensorBase::getType(void)
{
	return dsUnknown;
}

vDouble2 DistanceSensorBase::getRange(void)
{
	vDouble2 r;
	r.init();
	return r;
}


}
