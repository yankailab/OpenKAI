/*
 * DistSensorBase.h
 *
 *  Created on: Apr 13, 2017
 *      Author: yankai
 */

#include "DistSensorBase.h"

namespace kai
{

DistSensorBase::DistSensorBase()
{
	m_range.x = 0.0;
	m_range.y = DBL_MAX;
}

DistSensorBase::~DistSensorBase()
{
}

DIST_SENSOR_TYPE DistSensorBase::type(void)
{
	return dsUnknown;
}

vDouble2 DistSensorBase::range(void)
{
	return m_range;
}

double DistSensorBase::d(void)
{
	return -1.0;
}

double DistSensorBase::d(vInt4* pROI, vInt2* pPos)
{
	return -1.0;
}

double DistSensorBase::d(vDouble4* pROI, vInt2* pPos)
{
	return -1.0;
}

double DistSensorBase::d(int iSegment)
{
	return -1.0;
}

}
