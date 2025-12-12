#ifndef OpenKAI_src_Protocol__ProtocolBase_H_
#define OpenKAI_src_Protocol__ProtocolBase_H_

#include "../Base/_ModuleBase.h"
#include "../IO/_IObase.h"

// 0 PROTOCOL_BEGIN
// 1 COMMAND
// 2 PAYLOAD LENGTH H
// 3 PAYLOAD LENGTH L
// 4 Payload...

#define PB_BEGIN 0xFE
#define PB_N_HDR 4
#define PB_N_BUF 2000

namespace kai
{
	struct PROTOCOL_CMD
	{
		uint8_t m_cmd;
		uint16_t m_nPayload;
		uint16_t m_iB;
		uint8_t m_pB[PB_N_BUF];

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

				if (m_iB == 4)
				{
					m_nPayload = *((uint16_t *)&m_pB[2]);
					//m_nPayload = unpack_int16(&m_pB[2], false);
				}

				if (m_iB == m_nPayload + PB_N_HDR)
				{
					m_cmd = m_pB[1];
					return true;
				}
			}
			else if (b == PB_BEGIN)
			{
				m_cmd = b;
				m_pB[0] = b;
				m_iB = 1;
				m_nPayload = 0;
			}

			return false;
		}
	};

	class _ProtocolBase : public _ModuleBase
	{
	public:
		_ProtocolBase();
		~_ProtocolBase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		virtual void send(void);
		virtual bool readCMD(PROTOCOL_CMD *pCmd);
		virtual void handleCMD(const PROTOCOL_CMD &cmd);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_ProtocolBase *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_ProtocolBase *)This)->updateR();
			return NULL;
		}

	protected:
		_Thread *m_pTr;
		_IObase *m_pIO;
		uint64_t m_nCMDrecv;

		uint8_t m_pBuf[PB_N_BUF];
		int m_nRead;
		int m_iRead;
	};

}
#endif
