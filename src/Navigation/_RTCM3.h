/*
 * _RTCM3.h
 *
 *  Created on: Jun 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation__RTCM3_H_
#define OpenKAI_src_Navigation__RTCM3_H_

#include "../Protocol/_ProtocolBase.h"

// 0 RTCM BEGIN 0xD3
// 1 Length H
// 2 Length L
// 3 Payload...
// 3-byte CRC24Q

#define RTCM_BEGIN 0xFE
#define RTCM_HDR 3
#define RTCM_CRC 3
#define RTCM_N_BUF 1030

namespace kai
{
	struct RTCM_BUF
	{
		uint8_t m_cmd;
		uint16_t m_nPayload;
		uint16_t m_iB;
		uint8_t m_pB[RTCM_N_BUF];

		void clear(void)
		{
			m_cmd = 0;
			m_nPayload = 0;
			m_iB = 0;
		}

		bool input(uint8_t b)
		{
			if (m_cmd != 0)
			{
				m_pB[m_iB++] = b;

				if (m_iB == 2)
				{
					m_nPayload = *((uint16_t *)&m_pB[1]);
					// m_nPayload = unpack_int16(&m_pB[2], false);
				}

				if (m_iB == m_nPayload + RTCM_HDR + RTCM_CRC)
				{
					m_cmd = m_pB[1];
					return true;
				}
			}
			else if (b == RTCM_BEGIN)
			{
				m_cmd = b;
				m_pB[0] = b;
				m_iB = 1;
				m_nPayload = 0;
			}

			return false;
		}
	};

	class _RTCM3 : public _ProtocolBase
	{
	public:
		_RTCM3(void);
		virtual ~_RTCM3();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		virtual void send(void);
		virtual bool readCMD(RTCM_BUF *pCmd);
		virtual void handleCMD(const RTCM_BUF &cmd);

	private:
		void decode(void);
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_RTCM3 *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_RTCM3 *)This)->updateR();
			return NULL;
		}

	protected:
	};

}
#endif
