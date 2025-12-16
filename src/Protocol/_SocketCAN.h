#ifndef OpenKAI_src_Protocol__SocketCAN_H_
#define OpenKAI_src_Protocol__SocketCAN_H_

#include "../Base/_ModuleBase.h"
#include "../IO/_IObase.h"
#include "../Utility/util.h"

#include <linux/can.h>
#include <linux/can/raw.h>

namespace kai
{
	class _SocketCAN : public _ModuleBase
	{
	public:
		_SocketCAN();
		~_SocketCAN();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		virtual bool open(void);
		virtual bool bOpen(void);
		virtual void close(void);

		virtual bool sendFrame(const can_frame& f, int nW = sizeof(can_frame));

	protected:
		virtual bool readFrame(can_frame *pF, int nB = sizeof(can_frame));
		virtual void handleFrame(const can_frame &f);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_SocketCAN *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_SocketCAN *)This)->updateR();
			return NULL;
		}

	protected:
		_Thread *m_pTr;

		string m_ifName;
		int m_socket;

		bool m_bOpen;
		uint64_t m_nFrameRecv;
	};

}
#endif
