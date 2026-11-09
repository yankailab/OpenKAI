#ifndef OpenKAI_src_Net__HttpServer_H_
#define OpenKAI_src_Net__HttpServer_H_

#include "../Base/_ModuleBase.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace kai
{
	class HttpServer;

	// JSON-configurable static files, served by HttpServer's own IO worker.
	class _HttpServer : public _ModuleBase
	{
	public:
		_HttpServer();
		~_HttpServer() override;
		bool loadConfig() override;
		bool saveConfig(bool bExport) override;
		bool start() override;
		void stop() override;
		bool bRun() override;
		bool bRunning() override;
		bool bStopped() override;

	private:
		void update() override;
		string m_host = "127.0.0.1";
		string m_root = "html";
		int m_port = 80;
		int m_maxClients = 32;
		std::unique_ptr<HttpServer> m_http;
		std::thread m_worker;
		std::atomic<bool> m_running{false};
		std::mutex m_waitMutex;
		std::condition_variable m_wakeup;
	};
}
#endif
