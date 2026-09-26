#ifndef OpenKAI_src_UI_Viewer_Web__WebGeometryBase_H_
#define OpenKAI_src_UI_Viewer_Web__WebGeometryBase_H_

#include "../../../Universe/Geometry/_GeometryViewerBase.h"
#include "WebSelectableOctGridProtocol.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <thread>

namespace kai
{
	class HttpServer;
	class WebSocketStream;

	class _WebGeometryBase : public _GeometryViewerBase
	{
	public:
		_WebGeometryBase();
		~_WebGeometryBase() override;
		bool loadConfig(void) override;
		bool link(void) override;
		bool start() override;
		bool bRun() override { return m_running; }
		bool bRunning() override { return m_running && !m_paused; }
		bool bStopped() override { return !m_running; }
		void stop() override;
		void pause() override;
		void resume() override;
		void console(void *pConsole) override;

	protected:
		void updateAllGeometries() override;

	private:
		struct Stream
		{
			webselectableoctgrid::Type type = webselectableoctgrid::Type::Points;
			std::unique_ptr<WebSocketStream> transport;
			std::vector<std::shared_ptr<std::vector<uint8_t>>> buffers;
			uint32_t sequence = 0;
			std::atomic<size_t> bytes{0};
		};
		struct GeometryStyle
		{
			std::string m_name;
			int m_nP = 0, m_nL = 0;
			bool m_bVisible = true;
			Vector4f m_matCol{1, 1, 1, 1};
			float m_matPointSize = 2;
		};
		bool includes(const GeometryStyle &object, webselectableoctgrid::Type type) const;
		void collectGeometry(_GeometryBase *geometry, const GeometryStyle &object, webselectableoctgrid::Type type, std::vector<uint8_t> &frame, uint32_t id, uint64_t expiry);
		void publish(Stream &stream);
		std::string hello(webselectableoctgrid::Type type) const;
		std::string m_host = "0.0.0.0", m_root = "html/viewer/_GeometryBase";
		int m_port = 8080, m_maxClients = 8;
		Vector4f m_background{0.035f, 0.045f, 0.065f, 1};
		bool m_autoBound = true, m_showGrid = true;
		bool m_bFrame = false;
		std::vector<_GeometryBase *> m_vGeometry;
		std::vector<GeometryStyle> m_styles;
		std::vector<Vector3f> m_framePositions, m_frameColors;
		std::unique_ptr<HttpServer> m_http;
		std::array<Stream, 2> m_streams;
		std::thread m_worker;
		std::atomic<bool> m_running{false}, m_paused{false};
		std::mutex m_waitMutex;
		std::condition_variable m_wakeup;
		std::vector<float> m_positions;
		std::vector<uint8_t> m_colors;
	};
}
#endif
