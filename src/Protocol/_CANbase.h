#ifndef OpenKAI_src_Protocol__CANbase_H_
#define OpenKAI_src_Protocol__CANbase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"

#define CAN_BUF_N 64

namespace kai
{
	struct CAN_F
	{
		uint32_t m_ID;
		uint8_t m_pData[CAN_BUF_N];
		uint8_t m_nData;
		bool m_bExtended;
		bool m_bRTR;

		void clear(void)
		{
			m_ID = 0;
			memset(m_pData, 0, CAN_BUF_N);
			m_nData = 0;
			m_bExtended = false;
			m_bRTR = false;
		}
	};

	class _CANbase : public _ModuleBase
	{
	public:
		_CANbase();
		~_CANbase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		virtual bool open(void);
		virtual bool bOpen(void);
		virtual void close(void);

		virtual bool sendFrame(const CAN_F& f);
		virtual bool readFrame(CAN_F* pF);
		virtual void handleFrame(const CAN_F &f);

	protected:
		bool m_bOpen;
		uint64_t m_nFrameRecv;

		int m_nErrReconnect;	// reconnect on n-th error
		int m_iErr;
	};

}
#endif
