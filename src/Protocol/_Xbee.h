#ifndef OpenKAI_src_Protocol__Xbee_H_
#define OpenKAI_src_Protocol__Xbee_H_

#include "../Base/_ModuleBase.h"
#include "../IO/_IOBase.h"
#include "../Utility/util.h"

#define XB_DELIM 0x7E
#define XB_N_FRAME 128
#define XB_N_PAYLOAD 49

namespace kai
{
	struct XBframe_transitRequest
	{
		// buffer
		uint8_t m_pF[XB_N_FRAME];
		uint16_t m_nF;

		// header
		uint16_t m_length;

		// API ID
		uint8_t m_fType = 0x10;
		uint8_t m_fID = 1;
		uint64_t m_destAddr = 0x000000000000FFFF; // Broadcast address
		uint16_t m_destAddr16 = 0xFFFE;
		uint8_t m_brRadius = 0;
		uint8_t m_options = 0;
		uint8_t m_pD[XB_N_PAYLOAD];
		uint8_t m_nD;

		bool encode(uint8_t *pD, int nD)
		{
			IF_F(nD > XB_N_PAYLOAD);
			NULL_F(pD);

			// copy data
			memcpy(m_pD, pD, nD);
			m_nD = nD;
			m_length = 14 + m_nD;
			m_nF = m_length + 4;

			// Header
			m_pF[0] = XB_DELIM;
			m_pF[1] = m_length >> 8;
			m_pF[2] = m_length & 0xFF;

			// Transimit request
			m_pF[3] = m_fType;
			m_pF[4] = m_fID;
			*((uint64_t *)(&m_pF[5])) = swap_uint64(m_destAddr);
			*((uint16_t *)(&m_pF[13])) = swap_uint16(m_destAddr16);
			m_pF[15] = m_brRadius;
			m_pF[16] = m_options;
			memcpy(&m_pF[17], m_pD, m_nD);
			m_pF[17 + m_nD] = checksum(&m_pF[3], m_length);

			// uint32_t s = 0;
			// int nS = m_length + 3;
			// for (int i=3; i < nS; i++)
			// {
			// 	s += m_pF[i];
			// }
			// s &= 0xFF;
			// s = 0xFF - s;

			return true;
		}
	};

	struct XBframe_receivePacket
	{
		// header
		uint16_t m_length;

		// API ID
		uint8_t m_fType = 0x90;
		uint64_t m_srcAddr = 0x000000000000FFFF;
		uint16_t m_srcAddr16 = 0xFFFE;
		uint8_t m_options = 0;
		uint8_t m_pD[XB_N_PAYLOAD];
		uint8_t m_nD;

		bool decode(uint8_t *pB, int nB)
		{
			IF_F(nB > XB_N_PAYLOAD);
			NULL_F(pB);

			// Header
			m_length = (pB[1] << 8) | pB[2];
			IF_F(m_length + 4 > nB);

			// Receive Packet
			IF_F(pB[3] != m_fType);
			m_srcAddr = swap_uint64(*((uint64_t *)(&pB[4])));
			m_srcAddr16 = swap_uint16(*((uint16_t *)(&pB[12])));
			m_options = pB[14];
			m_nD = m_length - 12;
			memcpy(m_pD, &pB[15], m_nD);

			uint8_t s = checksum(&pB[3], m_length);
			IF_F(pB[m_length + 3] != s);

			return true;
		}
	};

	struct XBframe_buf
	{
		int m_length;
		int m_iB;
		uint8_t m_pB[XB_N_FRAME];

		void reset(void)
		{
			m_length = 0;
			m_iB = 0;
		}
	};

	typedef void (*CbXBeeReceivePacket)(void *pInst, XBframe_receivePacket d);
	struct XBcb_receivePacket
	{
		CbXBeeReceivePacket m_pCb = NULL;
		void *m_pInst = NULL;

		void set(CbXBeeReceivePacket pCb, void *pPinst)
		{
			m_pCb = pCb;
			m_pInst = pPinst;
		}

		bool bValid(void)
		{
			return (m_pCb && m_pInst) ? true : false;
		}

		void call(XBframe_receivePacket d)
		{
			if (!bValid())
				return;

			m_pCb(m_pInst, d);
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

		virtual uint64_t getMyAddr(void);
		virtual uint64_t getAddr(const string &sAddr);
		virtual bool setCbReceivePacket(CbXBeeReceivePacket pCb, void *pInst);

	protected:
		virtual void send(const string &dest, uint8_t *pB, int nB);
		virtual void updateMesh(void);

		virtual bool recv(void);
		virtual void handleFrame(void);

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

	protected:
		_Thread *m_pTr;
		_IOBase *m_pIO;

		uint64_t m_myAddr;

		XBframe_buf m_fRecv;
		XBcb_receivePacket m_cbReceivePacket;
	};

}
#endif
