/*
 * CANframeStream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__CANframeStream__H_
#define OpenKAI_src__DataStream__CANframeStream__H_

#include "DataObjStream.h"

#define CAN_FRAME_BUF_N 64

namespace kai
{
	struct CAN_FRAME
	{
		uint32_t m_ID = 0;
		uint8_t m_pData[CAN_FRAME_BUF_N]{};
		uint8_t m_nData = 0;
		bool m_bExtended = false;
		bool m_bRTR = false;
		uint64_t m_tStamp = 0;

		void clear(void)
		{
			m_ID = 0;
			memset(m_pData, 0, CAN_FRAME_BUF_N);
			m_nData = 0;
			m_bExtended = false;
			m_bRTR = false;
			m_tStamp = 0;
		}
	};

	class CANframeStream : public DataObjStream<CAN_FRAME>
	{
	public:
		CANframeStream();
		virtual ~CANframeStream();
		void console(void *pConsole) override;

		bool loadConfig(void);
		bool saveConfig(bool bExport = false);

		bool clear(size_t nBuf);

	protected:
		uint32_t m_nBuf = 1000;
	};

}
#endif
