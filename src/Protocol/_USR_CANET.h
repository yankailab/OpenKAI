#ifndef OpenKAI_src_Protocol__USR_CANET_H_
#define OpenKAI_src_Protocol__USR_CANET_H_

#include "../Base/_ModuleBase.h"
#include "../IO/_IObase.h"
#include "../Utility/util.h"

#define CAN_BUF_N 13

namespace kai
{
	struct CAN_FRAME
	{
		// frame
		uint8_t m_ctrl;
		uint32_t m_ID;
		uint8_t m_pData[8];

		uint8_t m_nData;
		bool m_bExtended;
		bool m_bRTR;

		// working buffer
		uint8_t m_pB[CAN_BUF_N];
		uint8_t m_iB;


		bool read(_IObase *pIO)
		{
			NULL_F(pIO);

			int nR = pIO->read(&m_pB[m_iB], CAN_BUF_N - m_iB);
			IF_F(nR <= 0);
			m_iB += nR;

			IF_F(m_iB < CAN_BUF_N);

			decode();
			m_iB = 0;
			return true;
		}

		void decode(void)
		{
			m_ctrl = m_pB[0];
			m_nData = m_ctrl & 0x0F;
			m_bExtended = (m_ctrl & (1 << 7));
			m_bRTR = (m_ctrl & (1 << 6));

			m_ID = *((uint32_t *)&m_pB[1]);
			memcpy(m_pData, &m_pB[5], 8);
		}

		bool encode(uint32_t ID, uint8_t nData, uint8_t *pData, bool bExtended = false, bool bRTR = false)
		{
			IF_F(nData > 8);
			NULL_F(pData);

			m_ID = ID;
			m_nData = nData;
			m_bExtended = bExtended;
			m_bRTR = bRTR;

			m_ctrl = 0;
			m_ctrl |= (nData & 0x0F);
			if (bExtended)
			{
				m_ctrl |= (1 << 7);
			}
			if (bRTR)
			{
				m_ctrl |= (1 << 6);
			}

			// clear buf
			memset(m_pB, 0, CAN_BUF_N);

			// ctrl byte
			m_pB[0] = m_ctrl;

			// ID
			pack_uint32(&m_pB[1], m_ID, true);

			// data
			memcpy(&m_pB[5], pData, nData);

			return true;
		}
	};

	class _USR_CANET : public _ModuleBase
	{
	public:
		_USR_CANET();
		~_USR_CANET();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		virtual bool sendFrame(CAN_FRAME* pF);

	protected:
		virtual bool readFrame(CAN_FRAME *pFrame);
		virtual void handleFrame(const CAN_FRAME &frame);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_USR_CANET *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_USR_CANET *)This)->updateR();
			return NULL;
		}

	protected:
		_Thread *m_pTr;
		_IObase *m_pIO;
		uint64_t m_nFrameRecv;
	};

}
#endif
