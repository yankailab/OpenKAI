#ifndef OpenKAI_src_UI_Viewer_Web__WebGLIM_H_
#define OpenKAI_src_UI_Viewer_Web__WebGLIM_H_

#include "../../../Universe/Geometry/_GeometryViewerBase.h"
#include "WebGLIMProtocol.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace kai
{
	class _GLIM;
	class _WebGLIM : public _GeometryViewerBase
	{
	public:
		_WebGLIM();
		~_WebGLIM() override;
		bool init(const json &j) override;
		bool link(const json &j, ModuleMgr *manager) override;
		bool start() override;
		void stop() override;
		void pause() override;
		void resume() override;
		bool bRun() override { return m_running; }
		bool bRunning() override { return m_running && !m_paused; }
		bool bStopped() override { return !m_running; }
		void console(void *console) override;
	private:
		void publish();
		std::string hello() const;
		_GLIM *m_slam = nullptr;
		std::string m_host = "0.0.0.0", m_root = "html/viewer/_GLIM";
		int m_port = 8080, m_maxClients = 8;
		float m_pointSize = 2;
		Vector4f m_background{0.035f, 0.045f, 0.065f, 1};
		bool m_autoBound = true, m_showGrid = true;
		uint64_t m_session = UINT64_MAX, m_revision = UINT64_MAX;
		std::unique_ptr<HttpServer> m_http;
		std::unique_ptr<webglim::Stream> m_stream;
		std::thread m_worker;
		std::atomic<bool> m_running{false}, m_paused{false};
		std::mutex m_waitMutex;
		std::condition_variable m_wakeup;
	};
}
#endif
