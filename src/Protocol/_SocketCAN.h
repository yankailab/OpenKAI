#ifndef OpenKAI_src_Protocol__SocketCAN_H_
#define OpenKAI_src_Protocol__SocketCAN_H_

#include "../Base/_ModuleBase.h"
#include "../DataObject/CANframeStream.h"
#include <atomic>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <shared_mutex>

namespace kai
{
	class _SocketCAN : public _ModuleBase
	{
	public:
		_SocketCAN();
		~_SocketCAN() override;

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool start(void) override;
		bool check(void) override;
		void console(void *pConsole) override;
		void pause(void) override;
		void resume(void) override;
		void stop(void) override;
		bool bRun(void) override;
		bool bRunning(void) override;
		bool bStopped(void) override;

		virtual bool open(void);
		virtual bool bOpen(void);
		virtual void close(void);
		virtual bool sendFrame(void);
		virtual bool readFrame(void);

	protected:
		_Thread *getThread(const string &name) override;

	private:
		void updateW(void);
		static void *getUpdateW(void *pThis)
		{
			static_cast<_SocketCAN *>(pThis)->updateW();
			return nullptr;
		}

		void updateR(void);
		static void *getUpdateR(void *pThis)
		{
			static_cast<_SocketCAN *>(pThis)->updateR();
			return nullptr;
		}

	protected:
		// m_pT sends queued frames; m_pTr receives frames from the socket.
		_Thread *m_pTr = nullptr;
		CANframeStream *m_pCANframeIn = nullptr;
		uint64_t m_tLastCANframeIn = 0;
		vector<CAN_FRAME> m_vFrameIn;
		size_t m_iFrameIn = 0;
		CANframeStream *m_pCANframeOut = nullptr;
		uint64_t m_tLastCANframeOut = 0;

		string m_ifName = "can0";
		int m_socket = -1;
		// Syscalls hold shared locks; open/close hold an exclusive lock.
		std::shared_mutex m_connectionMutex;
		std::atomic<bool> m_bOpened{false};
		std::atomic<uint64_t> m_nFrameRecv{0};
		int m_nErrReconnect = 1;
		std::atomic<int> m_iErr{0};
	};

}
#endif
