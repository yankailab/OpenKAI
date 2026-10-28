#ifndef OpenKAI_src_Net__HttpServer_H_
#define OpenKAI_src_Net__HttpServer_H_

#include "../Base/_ModuleBase.h"
#include <memory>

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
		string m_host = "127.0.0.1";
		string m_root = "html";
		int m_port = 80;
		int m_maxClients = 32;
		std::unique_ptr<HttpServer> m_http;
	};
}
#endif
