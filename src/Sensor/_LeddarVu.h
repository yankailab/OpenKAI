/*
 * _LeddarVu.h
 *
 *  Created on: Apr 3, 2017
 *      Author: yankai
 */

#ifndef SENSOR__LeddarVu_H_
#define SENSOR__LeddarVu_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../include/libmodbus/modbus.h"

#define N_SEGMENT 8
#define LEDDAR_MAX_DETECTIONS 16

namespace kai
{

using std::string;
using namespace std;
using namespace cv;

struct SDetection
{
	uint8_t channel;			 // Channel number (0..15)
	double dDistance;			 // distance from the sensor, in meters
	double dAmplitude;			 // signal amplitude
	uint8_t flags;               // Flags
};

class _LeddarVu: public _ThreadBase
{
public:
	_LeddarVu();
	~_LeddarVu();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	void reset(void);

	double getDistance(double localAngle);
	double minDist(void);
	double maxDist(void);

private:
	bool SetSlaveAddr(modbus_t* mb, int iSlaveAddr);
//	bool ReadDetectionsFast(modbus_t* mb, SDetection tabDetections[LEDDAR_MAX_DETECTIONS],
//							       uint32_t& nbrDetections, uint32_t& uTimestamp);
	bool ReadDetectionsReg(modbus_t* mb, SDetection tabDetections[LEDDAR_MAX_DETECTIONS],
							      uint32_t& nbrDetections, uint32_t& uTimestamp);
	void TestConnection(modbus_t* mb);




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
	double	m_minDist;
	double	m_maxDist;

};

}

#endif
