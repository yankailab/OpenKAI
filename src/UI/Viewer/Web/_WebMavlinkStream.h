#ifndef OpenKAI_src_UI_Viewer_Web__WebMavlinkStream_H_
#define OpenKAI_src_UI_Viewer_Web__WebMavlinkStream_H_

#include "../../../Base/_ModuleBase.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace kai
{
	class HttpServer;
	class WebSocketStream;
	class MavlinkStream;

	// A read-only MAVLink telemetry projection, independent of geometry protocols.
	class _WebMavlinkStream : public _ModuleBase
	{
	public:
		_WebMavlinkStream();
		~_WebMavlinkStream() override;
		bool loadConfig() override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool start() override;
		bool check() override;
		bool bRun() override { return m_running; }
		bool bRunning() override { return m_running && !m_paused; }
		bool bStopped() override { return !m_running; }
		void stop() override;
		void pause() override;
		void resume() override;
		void console(void *pConsole) override;

	private:
		void publish();
		MavlinkStream *m_stream = nullptr;
		std::string m_streamName;
		std::string m_host = "0.0.0.0";
		std::string m_root = "html/viewer/mavlink";
		std::string m_modelsRoot = "/home/kai/dev/models/webMavlink";
		int m_port = 8080, m_maxClients = 8;
		uint64_t m_staleAfterMs = 3000, m_sequence = 0;
		json m_scene = json::object();
		std::unique_ptr<HttpServer> m_http;
		std::unique_ptr<WebSocketStream> m_transport;
		std::thread m_worker;
		std::atomic<bool> m_running{false}, m_paused{false};
		std::atomic<size_t> m_frameBytes{0};
		std::mutex m_waitMutex;
		std::condition_variable m_wakeup;
	};
}
#endif
