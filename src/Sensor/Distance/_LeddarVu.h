/*
 * _LeddarVu.h
 *
 *  Created on: Apr 3, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__LeddarVu_H_
#define OpenKAI_src_Sensor__LeddarVu_H_

#include "../../Dependencies/libmodbus/modbus.h"
#include "_DistSensorBase.h"

#define LEDDAR_MAX_DETECTIONS 16

namespace kai
{

	class _LeddarVu : public _DistSensorBase
	{
	public:
		_LeddarVu();
		~_LeddarVu();

		virtual bool init(const json &j);
		virtual bool start(void);
		virtual void draw(void *pFrame);
		virtual void console(void *pConsole);

		DIST_SENSOR_TYPE type(void);

	private:
		bool open(void);
		bool updateLidar(void);
		bool updateLidarFast(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_LeddarVu *)This)->update();
			return NULL;
		}

	protected:
		modbus_t *m_pMb = nullptr;
		string m_port = "";
		int m_baud = 115200;
		// Current slave address being used (because modbus_send_raw_request() doesn't use the address set
		// by modbus_set_slave()):
		int m_slaveAddr = 1;
		// True to use command 0x41 to fetch data; otherwise, we use the standard Modbus registers:
		bool m_bUse0x41 = false;
		float m_showOriginOffsetX = 0.5;
		float m_showOriginOffsetY = 0.5;

		uint16_t m_nDetection = 0;

		// configurations
		uint16_t m_nAccumulationsExpo = 5;
		uint16_t m_nOversamplingsExpo = 1;
		uint16_t m_lightSrcPwr = 100;
		uint16_t m_nPoint = 18;
		uint16_t m_oprMode = 1;
		bool m_bAutoLightSrcPwr = false;
		bool m_bDemergeObj = true;
		bool m_bStaticNoiseRemoval = true;
		bool m_bPrecision = true;
		bool m_bSaturationCompensation = true;
		bool m_bOvershootManagement = true;
	};

}
#endif
