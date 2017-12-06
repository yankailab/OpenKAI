/*
 * _RPLIDAR.h
 *
 *  Created on: Apr 3, 2017
 *      Author: yankai
 */

#ifndef SENSOR__RPLIDAR_H_
#define SENSOR__RPLIDAR_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../DistSensorBase.h"
#include "sdk/include/rplidar.h"

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif


namespace kai
{
using namespace rp::standalone::rplidar;

class _RPLIDAR: public _ThreadBase, public DistSensorBase
{
public:
	_RPLIDAR();
	~_RPLIDAR();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
//	void reset(void);

	double getDistance(double localAngle);

	DIST_SENSOR_TYPE type(void);
	vDouble2 range(void);

private:
	bool open(void);
	bool checkRPLIDARHealth(RPlidarDriver * drv);
	bool updateLidar(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_RPLIDAR*) This)->update();
		return NULL;
	}

public:
	string	m_portName;
	int		m_baud;

	int		m_nDiv;
	double	m_dAngle;
	double	m_angleV;
	double	m_angleH;
	double	m_dMin;
	double	m_dMax;

	uint16_t m_bReady;
	uint16_t m_nDetection;
	uint32_t m_timeStamp;

};

}

#endif
