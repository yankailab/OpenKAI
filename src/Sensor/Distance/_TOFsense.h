/*
 * _TOFsense.h
 *
 *  Created on: July 15, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Sensor__TOFsense_H_
#define OpenKAI_src_Sensor__TOFsense_H_

#include "../../Base/_ModuleBase.h"
#include "../../DataObject/PCLframe.h"
#include "../../DataObject/BytePacketStream.h"

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
		uint8_t *m_pB = nullptr;

		void init(int nB)
		{
			DEL_ARRAY(m_pB);
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

	class _TOFsense : public _ModuleBase
	{
	public:
		_TOFsense();
		~_TOFsense();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		virtual void console(void *pConsole);

		int32_t int24Value(uint8_t *pD);
		uint8_t verifyCheckSum(const uint8_t *data, size_t dataLength);

	private:
		bool readByte(uint8_t *pB);
		bool readCMD(void);
		void handleCMD(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_TOFsense *)This)->update();
			return NULL;
		}

	protected:
		PCLframe *m_pPCLout = nullptr;
		Vector2f m_vRange = Vector2f(0, FLT_MAX);
		int m_nDiv = 1;
		float m_fovH = 360;
		vector<GEOMETRY_POINT> m_vPoints;

		BytePacketStream *m_pBpStreamIn = nullptr;
		uint64_t m_tLastBpStreamIn = 0;
		vector<uint8_t> m_vBytesIn;
		size_t m_iByteIn = 0;
		NLINK_FRAME m_frame;
	};

}
#endif
