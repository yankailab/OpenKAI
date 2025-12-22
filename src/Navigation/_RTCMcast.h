/*
 * _RTCMcast.h
 *
 *  Created on: Jun 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation__RTCMcast_H_
#define OpenKAI_src_Navigation__RTCMcast_H_

#include "../Protocol/_ProtocolBase.h"
#include "../Utility/utilEvent.h"

// 0 RTCM BEGIN 0xD3
// 1 Length H
// 2 Length L
// 3 Payload...
// 3-byte CRC24Q

#define RTCM_BEGIN 0xD3
#define RTCM_HDR 3
#define RTCM_CRC 3
#define RTCM_N_BUF 1030

namespace kai
{
	struct RTCM_MSG
	{
		// rtcm msg data
		uint16_t m_msgID;
		uint16_t m_nPayload;
		uint16_t m_nB;
		uint8_t m_pB[RTCM_N_BUF];

		// control params
		INTERVAL_EVENT m_ieSend; // interval to send to clients
		TIME_OUT m_tOutRecv;	 // do not send after this timeout in receiving the msg
		uint64_t m_tLastRecv;
		uint64_t m_tLastSent;
		float m_tIntSec;
		bool m_bSendOnceOnly;
		int m_nRecv;

		void init(uint64_t ieSend = USEC_1SEC, uint64_t tOut = USEC_10SEC)
		{
			m_msgID = 0;
			m_nPayload = 0;
			m_nB = 0;

			m_ieSend.init(ieSend);
			m_ieSend.reset();

			m_tOutRecv.setTout(tOut);
			m_tOutRecv.reStartT(0);

			m_tLastRecv = 0;
			m_tLastSent = 0;
			m_tIntSec = -1;
			m_bSendOnceOnly = false;
			m_nRecv = 0;
		}

		bool input(uint8_t b)
		{
			if (m_nB > 0)
			{
				m_pB[m_nB++] = b;

				if (m_nB == 3)
				{
					m_nPayload = ((m_pB[1] & 0x03) << 8) | m_pB[2];
				}

				if (m_nB == m_nPayload + RTCM_HDR + RTCM_CRC)
				{
					// Message ID is first 12 bits of payload:
					// bits 0..7   = payload[0]
					// bits 8..11  = high 4 bits of payload[1]
					m_msgID = (((uint16_t)m_pB[3] << 4) | (m_pB[4] >> 4)) & 0x0FFF;

					return true;
				}
			}
			else if (b == RTCM_BEGIN)
			{
				m_pB[0] = b;
				m_nB = 1;
				m_nPayload = 0;
			}

			return false;
		}

		bool operator==(const RTCM_MSG &m)
		{
			IF_F(m_msgID != m.m_msgID);
			IF_F(m_nPayload != m.m_nPayload);
			IF_F(m_nB != m.m_nB);

			for (int i = 0; i < m_nB; i++)
			{
				IF_F(m_pB[i] != m.m_pB[i]);
			}

			return true;
		}

		bool updateTo(const RTCM_MSG &m)
		{
			IF_F(m_msgID != m.m_msgID);

			m_nPayload = m.m_nPayload;
			m_nB = m.m_nB;
			memcpy(m_pB, m.m_pB, m_nB);

			return true;
		}
	};

	class _RTCMcast : public _ProtocolBase
	{
	public:
		_RTCMcast(void);
		virtual ~_RTCMcast();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		virtual void sendMsg(void);
		virtual bool readMsg(RTCM_MSG *pMsg);
		virtual void handleMsg(const RTCM_MSG &msg);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_RTCMcast *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_RTCMcast *)This)->updateR();
			return NULL;
		}

	protected:
		_IObase *m_pIOsend;
		vector<RTCM_MSG> m_vMsg;
	};

}
#endif
