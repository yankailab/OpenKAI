/*
 * _RPLIDAR.h
 *
 *  Created on: Apr 3, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__RPLIDAR_H_
#define OpenKAI_src_Sensor__RPLIDAR_H_

#include "../_DistSensorBase.h"
#include "sdk/include/rplidar.h"

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

namespace kai
{
using namespace rp::standalone::rplidar;

class _RPLIDAR: public _DistSensorBase
{
public:
	_RPLIDAR();
	~_RPLIDAR();

	bool init(void* pKiss);
	bool start(void);

	DIST_SENSOR_TYPE type(void);

private:
	bool open(void);
	void close(void);
	bool checkRPLIDARHealth(void);
	void updateLidar(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_RPLIDAR*) This)->update();
		return NULL;
	}

public:
	string	m_port;
	int		m_baud;
	RPlidarDriver* m_pRPL;

};

}
#endif
