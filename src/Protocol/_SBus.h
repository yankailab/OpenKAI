#ifndef OpenKAI_src_Protocol__SBus_H_
#define OpenKAI_src_Protocol__SBus_H_

#include "_ProtocolBase.h"
#include "../Utility/RC.h"

// SBus definition
#define SBUS_HEADER_ 0x0F
#define SBUS_FOOTER_ 0x00
#define SBUS_FOOTER2_ 0x04
#define SBUS_CH17_ 0x01
#define SBUS_CH18_ 0x02
#define SBUS_LOST_FRAME_ 0x04
#define SBUS_FAILSAFE_ 0x08
#define SBUS_NCHAN 16

#define SBUS_N_BUF 35

namespace kai
{
	struct SBUS_FRAME
	{
		int m_iB;
		int m_nBframe = SBUS_N_BUF;
		uint8_t m_pB[SBUS_N_BUF];

		uint8_t m_flag;
		bool m_bLostFrame;
		bool m_bFailSafe;
		bool m_bCh17;
		bool m_bCh18;
		RC_CHANNEL m_pRC[SBUS_NCHAN];
		uint64_t m_timeOutUsec = USEC_1SEC;
		uint64_t m_tLastRecv;

		void clear(void)
		{
			m_iB = 0;

			m_flag = 0;
			m_bLostFrame = false;
			m_bFailSafe = false;
			m_bCh17 = false;
			m_bCh18 = false;
			m_tLastRecv = 0;
		}

		bool input(uint8_t b)
		{
			if (m_iB > 0)
			{
				m_pB[m_iB++] = b;
				IF__(m_iB == m_nBframe, true);
			}
			else if (b == SBUS_HEADER_)
			{
				m_pB[0] = b;
				m_iB = 1;
			}

			return false;
		}

		bool bChecksum(void)
		{
			//	uint8_t cs = checksum(&m_pB[1], 33);
			uint8_t cs = 0;
			for (int i = 1; i < 33; i++)
				cs ^= m_pB[i];

			return (m_pB[m_nBframe - 1] == cs);
		}

		bool bFailSafe(void)
		{
			IF__(m_bFailSafe, true);

			uint64_t t = getApproxTbootUs();
			IF_F(t <= m_tLastRecv);
			IF__(t - m_tLastRecv > m_timeOutUsec, true);

			return false;
		}

		void decodeRaw(void)
		{
			m_pRC[0].set(static_cast<uint16_t>(m_pB[1] | m_pB[2] << 8 & 0x07FF));
			m_pRC[1].set(static_cast<uint16_t>(m_pB[2] >> 3 | m_pB[3] << 5 & 0x07FF));
			m_pRC[2].set(static_cast<uint16_t>(m_pB[3] >> 6 | m_pB[4] << 2 |
											   m_pB[5] << 10 & 0x07FF));
			m_pRC[3].set(static_cast<uint16_t>(m_pB[5] >> 1 | m_pB[6] << 7 & 0x07FF));
			m_pRC[4].set(static_cast<uint16_t>(m_pB[6] >> 4 | m_pB[7] << 4 & 0x07FF));
			m_pRC[5].set(static_cast<uint16_t>(m_pB[7] >> 7 | m_pB[8] << 1 |
											   m_pB[9] << 9 & 0x07FF));
			m_pRC[6].set(static_cast<uint16_t>(m_pB[9] >> 2 | m_pB[10] << 6 & 0x07FF));
			m_pRC[7].set(static_cast<uint16_t>(m_pB[10] >> 5 | m_pB[11] << 3 & 0x07FF));
			m_pRC[8].set(static_cast<uint16_t>(m_pB[12] | m_pB[13] << 8 & 0x07FF));
			m_pRC[9].set(static_cast<uint16_t>(m_pB[13] >> 3 | m_pB[14] << 5 & 0x07FF));
			m_pRC[10].set(static_cast<uint16_t>(m_pB[14] >> 6 | m_pB[15] << 2 |
												m_pB[16] << 10 & 0x07FF));
			m_pRC[11].set(static_cast<uint16_t>(m_pB[16] >> 1 | m_pB[17] << 7 & 0x07FF));
			m_pRC[12].set(static_cast<uint16_t>(m_pB[17] >> 4 | m_pB[18] << 4 & 0x07FF));
			m_pRC[13].set(static_cast<uint16_t>(m_pB[18] >> 7 | m_pB[19] << 1 |
												m_pB[20] << 9 & 0x07FF));
			m_pRC[14].set(static_cast<uint16_t>(m_pB[20] >> 2 | m_pB[21] << 6 & 0x07FF));
			m_pRC[15].set(static_cast<uint16_t>(m_pB[21] >> 5 | m_pB[22] << 3 & 0x07FF));

			m_bLostFrame = m_pB[23] & SBUS_LOST_FRAME_;
			m_bFailSafe = m_pB[23] & SBUS_FAILSAFE_;
		}

		void decode(void)
		{
			for (int i = 0; i < 16; i++)
			{
				int j = i * 2;
				m_pRC[i].set(static_cast<uint16_t>(m_pB[j + 1] << 8 | m_pB[j + 2]));
			}

			m_flag = m_pB[SBUS_N_BUF - 2];
		}

		void encodeRaw(void)
		{
			m_pB[0] = SBUS_HEADER_;
			m_pB[1] = static_cast<uint8_t>((m_pRC[0].raw() & 0x07FF));
			m_pB[2] = static_cast<uint8_t>((m_pRC[0].raw() & 0x07FF) >> 8 |
										   (m_pRC[1].raw() & 0x07FF) << 3);
			m_pB[3] = static_cast<uint8_t>((m_pRC[1].raw() & 0x07FF) >> 5 |
										   (m_pRC[2].raw() & 0x07FF) << 6);
			m_pB[4] = static_cast<uint8_t>((m_pRC[2].raw() & 0x07FF) >> 2);
			m_pB[5] = static_cast<uint8_t>((m_pRC[2].raw() & 0x07FF) >> 10 |
										   (m_pRC[3].raw() & 0x07FF) << 1);
			m_pB[6] = static_cast<uint8_t>((m_pRC[3].raw() & 0x07FF) >> 7 |
										   (m_pRC[4].raw() & 0x07FF) << 4);
			m_pB[7] = static_cast<uint8_t>((m_pRC[4].raw() & 0x07FF) >> 4 |
										   (m_pRC[5].raw() & 0x07FF) << 7);
			m_pB[8] = static_cast<uint8_t>((m_pRC[5].raw() & 0x07FF) >> 1);
			m_pB[9] = static_cast<uint8_t>((m_pRC[5].raw() & 0x07FF) >> 9 |
										   (m_pRC[6].raw() & 0x07FF) << 2);
			m_pB[10] = static_cast<uint8_t>((m_pRC[6].raw() & 0x07FF) >> 6 |
											(m_pRC[7].raw() & 0x07FF) << 5);
			m_pB[11] = static_cast<uint8_t>((m_pRC[7].raw() & 0x07FF) >> 3);
			m_pB[12] = static_cast<uint8_t>((m_pRC[8].raw() & 0x07FF));
			m_pB[13] = static_cast<uint8_t>((m_pRC[8].raw() & 0x07FF) >> 8 |
											(m_pRC[9].raw() & 0x07FF) << 3);
			m_pB[14] = static_cast<uint8_t>((m_pRC[9].raw() & 0x07FF) >> 5 |
											(m_pRC[10].raw() & 0x07FF) << 6);
			m_pB[15] = static_cast<uint8_t>((m_pRC[10].raw() & 0x07FF) >> 2);
			m_pB[16] = static_cast<uint8_t>((m_pRC[10].raw() & 0x07FF) >> 10 |
											(m_pRC[11].raw() & 0x07FF) << 1);
			m_pB[17] = static_cast<uint8_t>((m_pRC[11].raw() & 0x07FF) >> 7 |
											(m_pRC[12].raw() & 0x07FF) << 4);
			m_pB[18] = static_cast<uint8_t>((m_pRC[12].raw() & 0x07FF) >> 4 |
											(m_pRC[13].raw() & 0x07FF) << 7);
			m_pB[19] = static_cast<uint8_t>((m_pRC[13].raw() & 0x07FF) >> 1);
			m_pB[20] = static_cast<uint8_t>((m_pRC[13].raw() & 0x07FF) >> 9 |
											(m_pRC[14].raw() & 0x07FF) << 2);
			m_pB[21] = static_cast<uint8_t>((m_pRC[14].raw() & 0x07FF) >> 6 |
											(m_pRC[15].raw() & 0x07FF) << 5);
			m_pB[22] = static_cast<uint8_t>((m_pRC[15].raw() & 0x07FF) >> 3);
			m_pB[23] = 0x00 | (m_bCh17 * SBUS_CH17_) | (m_bCh18 * SBUS_CH18_) |
					   (m_bFailSafe * SBUS_FAILSAFE_) | (m_bLostFrame * SBUS_LOST_FRAME_);
			m_pB[24] = SBUS_FOOTER_;
		}

		void encode(void)
		{
			for (int i = 0; i < 16; i++)
			{
				int j = i * 2;
				m_pB[j + 1] = static_cast<uint8_t>(m_pRC[i].raw() >> 8);
				m_pB[j + 2] = static_cast<uint8_t>(m_pRC[i].raw() & 0x00FF);
			}

			m_pB[34] = checksum(&m_pB[1], 33);
		}
	};

	class _SBus : public _ProtocolBase
	{
	public:
		_SBus();
		~_SBus();

		bool init(const json& j);
		bool link(const json& j, ModuleMgr* pM);
		bool start(void);
		bool check(void);
		void console(void *pConsole);

		uint16_t raw(int iChan);
		float v(int iChan);

	protected:
		virtual void send(void);
		virtual bool readSbus(SBUS_FRAME *pF);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_SBus *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_SBus *)This)->updateR();
			return NULL;
		}

	protected:
		bool m_bSender;
		bool m_bRawSbus;

		SBUS_FRAME m_frame;
	};

}
#endif
