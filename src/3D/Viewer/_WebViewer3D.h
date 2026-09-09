#ifndef OpenKAI_src_3D_Viewer__WebViewer3D_H_
#define OpenKAI_src_3D_Viewer__WebViewer3D_H_

#include "../_GeometryViewerBase.h"
#include "../Grid/OctreeGridCells.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <thread>

namespace kai
{
	class HttpServer;
	class WebSocketStream;

	class _WebViewer3D : public _GeometryViewerBase
	{
	public:
		_WebViewer3D();
		~_WebViewer3D() override;
		bool init(const json &j) override;
		bool link(const json &j, ModuleMgr *pM) override;
		bool start() override;
		bool bAlive() override { return m_running; }
		bool bRun() override { return m_running && !m_paused; }
		bool bStop() override { return !m_running; }
		void stop() override;
		void pause() override;
		void resume() override;
		void console(void *pConsole) override;

	protected:
		void updateAllGeometries() override;

	private:
		struct Object
		{
			_GeometryBase *source = nullptr;
			std::string name;
			int nP = 0, nL = 0, nC = 0;
			bool visible = true;
			float pointSize = 2;
			vFloat4 color{1, 1, 1, 1};
		};
		void collect(const Object &object, std::vector<uint8_t> &frame, uint32_t id, uint64_t expiry);
		std::string hello() const;
		std::string m_host = "0.0.0.0", m_root = "html/webViewer3D";
		int m_port = 8080, m_maxClients = 8, m_nCbuf = 100000;
		OCTGRID_CELLS m_cells;
		vFloat4 m_background{0.035f, 0.045f, 0.065f, 1};
		bool m_autoBound = true, m_showGrid = true;
		std::vector<Object> m_objects;
		std::unique_ptr<HttpServer> m_http;
		std::unique_ptr<WebSocketStream> m_stream;
		std::thread m_worker;
		std::atomic<bool> m_running{false}, m_paused{false};
		std::mutex m_waitMutex;
		std::condition_variable m_wakeup;
		uint32_t m_sequence = 0;
		std::atomic<size_t> m_frameBytes{0};
		std::vector<float> m_points, m_lines;
		std::vector<uint8_t> m_pointColors, m_lineColors;
		std::vector<std::shared_ptr<std::vector<uint8_t>>> m_buffers;
	};
}
#endif
