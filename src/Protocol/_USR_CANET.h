#ifndef OpenKAI_src_Protocol__USR_CANET_H_
#define OpenKAI_src_Protocol__USR_CANET_H_

#include "../Base/_ModuleBase.h"
#include "../IO/_IObase.h"

#define CAN_BUF_N 256

namespace kai
{
	struct CAN_FRAME
	{
		uint8_t m_frameInfo;
		uint8_t m_pID[4];
		uint8_t m_pData[8];

		void clear(void)
		{
			m_frameInfo = 0;
		}

		bool input(uint8_t b)
		{
			// if (m_cmd != 0)
			// {
			// 	m_pB[m_iB++] = b;

			// 	if (m_iB == 4)
			// 	{
			// 		m_nPayload = *((uint16_t *)&m_pB[2]);
			// 		//m_nPayload = unpack_int16(&m_pB[2], false);
			// 	}

			// 	if (m_iB == m_nPayload + PB_N_HDR)
			// 	{
			// 		m_cmd = m_pB[1];
			// 		return true;
			// 	}
			// }
			// else if (b == PB_BEGIN)
			// {
			// 	m_cmd = b;
			// 	m_pB[0] = b;
			// 	m_iB = 1;
			// 	m_nPayload = 0;
			// }

			return false;
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

	protected:
//		virtual void send(void);
		virtual void send(unsigned long addr, unsigned char len, unsigned char *pData);
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
		uint64_t m_nCMDrecv;

		uint8_t m_pBuf[CAN_BUF_N];
		int m_nRead;
		int m_iRead;
	};

}
#endif
