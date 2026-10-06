#ifndef OpenKAI_src_Protocol__USR_CANET_H_
#define OpenKAI_src_Protocol__USR_CANET_H_

#include "../Base/_ModuleBase.h"
#include "../DataObject/BytePacketStream.h"
#include "../DataObject/CANframeStream.h"
#include <atomic>

#define CANET_BUF_N 13

namespace kai
{

	class _USR_CANET : public _ModuleBase
	{
	public:
		_USR_CANET();
		~_USR_CANET() override;

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool start(void) override;
		bool check(void) override;
		void console(void *pConsole) override;

		bool bRun(void) override;
		bool bRunning(void) override;
		bool bStopped(void) override;
		void pause(void) override;
		void resume(void) override;
		void stop(void) override;

		virtual bool open(void);
		virtual bool bOpen(void);
		virtual void close(void);

		virtual bool sendFrame(void);
		virtual bool readFrame(void);

	protected:
		_Thread *getThread(const string &name) override;

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
		_Thread *m_pTr = nullptr;
		CANframeStream *m_pCANframeIn = nullptr;
		uint64_t m_tLastCANframeIn = 0;
		CANframeStream *m_pCANframeOut = nullptr;
		uint64_t m_tLastCANframeOut = 0;

		BytePacketStream *m_pBpStreamIn = nullptr;
		uint64_t m_tLastBpStreamIn = 0;
		BytePacketStream *m_pBpStreamOut = nullptr;
		vector<uint8_t> m_vFrameBytes;
		std::atomic<uint64_t> m_nFrameRecv{0};
	};

}
#endif
