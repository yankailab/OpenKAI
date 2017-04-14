/*
 * DistSensorBase.h
 *
 *  Created on: Apr 13, 2017
 *      Author: yankai
 */

#ifndef SRC_SENSOR_DISTSENSORBASE_H_
#define SRC_SENSOR_DISTSENSORBASE_H_

#include "../Base/common.h"

namespace kai
{

enum DIST_SENSOR_TYPE
{
	dsUnknown,
	dsZED,
	dsSF40,
	dsLeddarVu,
};

class DistSensorBase
{
public:
	DistSensorBase();
	virtual ~DistSensorBase();

	virtual DIST_SENSOR_TYPE type(void);
	virtual vDouble2 range(void);
	virtual double d(void);
	virtual double d(vInt4* pROI, vInt2* pPos);
	virtual double d(vDouble4* pROI, vInt2* pPos);
	virtual double d(int iSegment);

public:
	vDouble2	m_range;
};

}

#endif
