/*
 * _LeddarVu.h
 *
 *  Created on: Apr 3, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__LeddarVu_H_
#define OpenKAI_src_Sensor__LeddarVu_H_

#include "../include/libmodbus/modbus.h"
#include "../Vision/_VisionBase.h"
#include "_DistSensorBase.h"

#define N_SEGMENT 8
#define LEDDAR_MAX_DETECTIONS 16

namespace kai
{

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

	DIST_SENSOR_TYPE type(void);

private:
	bool open(void);
	bool updateLidar(void);
	bool updateLidarFast(void);
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
	_VisionBase* m_pVB;

	uint16_t m_nSegment;
	uint16_t m_nDetection;
	uint32_t m_tStamp;

	//configurations
	uint16_t	m_nAccumulationsExpo;
	uint16_t	m_nOversamplingsExpo;
	uint16_t	m_lightSrcPwr;
	bool		m_bAutoLightSrcPwr;
	bool		m_bDemergeObj;
	bool		m_bStaticNoiseRemoval;
	bool		m_bPrecision;
	bool		m_bSaturationCompensation;
	bool		m_bOvershootManagement;
	uint16_t	m_oprMode;

};

}

#endif
