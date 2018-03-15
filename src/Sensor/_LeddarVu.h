/*
 * _LeddarVu.h
 *
 *  Created on: Apr 3, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__LeddarVu_H_
#define OpenKAI_src_Sensor__LeddarVu_H_

#include "../include/libmodbus/modbus.h"
#include "_DistSensorBase.h"

#define N_SEGMENT 8
#define LEDDAR_MAX_DETECTIONS 16

namespace kai
{

using std::string;
using namespace std;
using namespace cv;

struct SEGMENT_DETECTION
{
	double dDistance;			 // distance from the sensor, in meters
	double dAmplitude;			 // signal amplitude
	uint16_t flags;              // Flags
};

class _LeddarVu: public _DistSensorBase
{
public:
	_LeddarVu();
	~_LeddarVu();

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
	bool updateLidar(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_LeddarVu*) This)->update();
		return NULL;
	}

public:
	modbus_t *m_pMb;
	string	m_portName;
	int		m_baud;
	// Current slave address being used (because modbus_send_raw_request() doesn't use the address set
	// by modbus_set_slave()):
	int m_slaveAddr;
	// True to use command 0x41 to fetch data; otherwise, we use the standard Modbus registers:
	bool m_bUse0x41;

	int		m_nDiv;
	double	m_dAngle;
	double	m_angleV;
	double	m_angleH;
	double	m_dMin;
	double	m_dMax;

	uint16_t m_bReady;
	uint16_t m_nSegment;
	uint16_t m_nDetection;
	uint16_t m_lightSrcPwr;
	uint32_t m_timeStamp;

	SEGMENT_DETECTION m_pSegment[N_SEGMENT];


};

}

#endif
