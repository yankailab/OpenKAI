/*
 * _IObase.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO_IObase_H_
#define OpenKAI_src_IO_IObase_H_

#include "../DataObject/BytePacketStream.h"

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"

namespace kai
{

	enum IO_STATUS
	{
		io_unknown,
		io_closed,
		io_opened
	};

	class _IObase : public _ModuleBase
	{
	public:
		_IObase();
		virtual ~_IObase();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual void console(void *pConsole);
		bool start(void) override;
		void pause(void) override;
		void resume(void) override;
		void stop(void) override;
		bool bRun(void) override;
		bool bRunning(void) override;
		bool bStopped(void) override;

		virtual bool open(void);
		virtual bool bOpen(void);
		virtual void close(void);

		virtual IO_STATUS getIOstatus(void);
		virtual void setIOstatus(IO_STATUS s);

	protected:
		_Thread *getThread(const string &name) override;
		virtual void readPackets(void);
		virtual void writePackets(void);
		virtual void updateW(void);
		virtual void updateR(void);

	private:
		static void *getUpdateW(void *pThis)
		{
			static_cast<_IObase *>(pThis)->updateW();
			return nullptr;
		}

		static void *getUpdateR(void *pThis)
		{
			static_cast<_IObase *>(pThis)->updateR();
			return nullptr;
		}

	protected:
		// m_pT consumes BytePacketStreamIn; m_pTr publishes BytePacketStreamOut.
		_Thread *m_pTr = nullptr;
		// Shared syscall locks permit concurrent read/write; open/close take exclusive locks.
		std::shared_mutex m_connectionMutex;
		std::atomic<uint64_t> m_connectionGeneration{0};

		BytePacketStream* m_pBpStreamIn = nullptr;	// read from this and write it out to the device
		uint64_t m_tLastBpStreamIn = 0;				// the last tStamp of the packet from m_pBpStreamIn's element written to the device, not the tStamp for BytePacketStream itself.

		BytePacketStream* m_pBpStreamOut = nullptr;	// read from device and put into this

		std::atomic<IO_STATUS> m_ioStatus{io_unknown};
	};

}
#endif
