#ifndef OpenKAI_src_Protocol__Xbee_H_
#define OpenKAI_src_Protocol__Xbee_H_

#include "../Base/_ModuleBase.h"
#include "../IO/_IOBase.h"
#include "../Utility/util.h"

#define XB_N_FRAME 128
#define XB_N_PAYLOAD 49

namespace kai
{
	struct XBframe_transitRequest
	{
		uint8_t m_pF[XB_N_FRAME];
		uint16_t m_nF;

		uint16_t m_length;
		uint8_t m_checksum;

		uint8_t m_fType = 0x10;
		uint8_t m_fID = 1;
		uint64_t m_destAddr = 0x000000000000FFFF; // Broadcast address
		uint16_t m_destAddr16 = 0xFFFE;
		uint8_t m_brRadius = 0;
		uint8_t m_options = 0;
		uint8_t m_pD[XB_N_PAYLOAD];
		uint8_t m_nD;

		bool setData(int nD, uint8_t *pD)
		{
			IF_F(nD > XB_N_PAYLOAD);

			memcpy(m_pD, pD, nD);
			m_nD = nD;
			m_length = 14 + m_nD;
			return true;
		}

		int encode(void)
		{
			m_pF[0] = 0x7E;
			m_pF[1] = m_length >> 8;
			m_pF[2] = m_length & 0xFF;

			m_pF[3] = m_fType;
			m_pF[4] = m_fID;
			*((uint64_t *)(&m_pF[5])) = swap_uint64(m_destAddr);
			*((uint16_t *)(&m_pF[13])) = swap_uint16(m_destAddr16);
			m_pF[15] = m_brRadius;
			m_pF[16] = m_options;
			memcpy(&m_pF[17], m_pD, m_nD);

			uint32_t s = 0;
			int nS = m_length + 3;
			for (int i=3; i < nS; i++)
			{
				s += m_pF[i];
			}
			s &= 0xFF;
			s = 0xFF - s;
			m_pF[17 + m_nD] = s;

			m_nF = m_length + 4;
			return m_nF;
		}
	};

	class _Xbee : public _ModuleBase
	{
	public:
		_Xbee();
		~_Xbee();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		uint64_t getAddr(const string &sAddr);

	protected:
		virtual void send(void);
		virtual bool recv(void);
		virtual void decFrame(uint8_t *pB);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_Xbee *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_Xbee *)This)->updateR();
			return NULL;
		}

	public:
		_Thread *m_pTr;
		_IOBase *m_pIO;

		string m_destAddr;
	};

}
#endif
