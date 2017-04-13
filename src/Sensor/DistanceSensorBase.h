/*
 * _DistanceSensorBase.h
 *
 *  Created on: Apr 13, 2017
 *      Author: yankai
 */

#ifndef SRC_SENSOR_DISTANCESENSORBASE_H_
#define SRC_SENSOR_DISTANCESENSORBASE_H_

#include "../Base/common.h"

namespace kai
{

enum DISTANCE_SENSOR_TYPE
{
	dsUnknown,
	dsZED,
	dsSF40,
	dsLeddarVu,
};

class DistanceSensorBase
{
public:
	DistanceSensorBase();
	virtual ~DistanceSensorBase();

	virtual DISTANCE_SENSOR_TYPE getType(void);
	virtual vDouble2 getRange(void);
	virtual uint8_t getOrientation(void);

public:
	uint8_t	m_orientation;

};

}

#endif
