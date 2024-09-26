/*
 * _TOFsense.h
 *
 *  Created on: July 15, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__TOFsense_H_
#define OpenKAI_src_Sensor__TOFsense_H_

#include "_DistSensorBase.h"
#include "../../IO/_IObase.h"

#define NLINK_BEGIN 0x57

namespace kai
{
	typedef struct
	{
		uint8_t byteArray[3];
	} NInt24;

	struct NLINK_FRAME
	{
		uint8_t m_header;
		int m_iB;
		uint8_t *m_pB;

		void init(int nB)
		{
			reset();

			if (nB <= 0)
				nB = 256;
			m_pB = new uint8_t[nB];
		}

		void reset(void)
		{
			m_header = 0;
			m_iB = 0;
		}
	};

	struct TOFSENSE_SENSOR
	{
		uint8_t header[2];
		uint8_t reserved0;
		uint8_t id;
		uint32_t systemTime;
		NInt24 distance;
		uint8_t distanceStatus;
		uint16_t signalStrength;
		uint8_t reserved1;
		uint8_t checkSum;
	};

	class _TOFsense : public _DistSensorBase
	{
	public:
		_TOFsense();
		~_TOFsense();

		int init(void *pKiss);
		int start(void);
		void console(void *pConsole);

		DIST_SENSOR_TYPE type(void);
		int32_t int24Value(uint8_t *pD);
		uint8_t verifyCheckSum(const uint8_t *data, size_t dataLength);

	private:
		bool readCMD(void);
		void handleCMD(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_TOFsense *)This)->update();
			return NULL;
		}

	private:
		_IObase *m_pIO;
		NLINK_FRAME m_frame;
	};

}
#endif
