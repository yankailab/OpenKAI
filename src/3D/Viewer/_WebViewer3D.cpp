#include "_WebViewer3D.h"
#include "WebViewer3DProtocol.h"
#include "../Grid/_OctreeGrid.h"
#include "../../Module/ModuleMgr.h"
#include "../../IO/WebSocketStream.h"
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace kai
{
	_WebViewer3D::_WebViewer3D() : m_http(new HttpServer) {}
	_WebViewer3D::~_WebViewer3D() { stop(); }

	bool _WebViewer3D::init(const json &j)
	{
		IF_F(!_GeometryViewerBase::init(j));
		jKv(j, "host", m_host);
		jKv(j, "port", m_port);
		jKv(j, "webRoot", m_root);
		jKv(j, "nClientMax", m_maxClients);
		jKv(j, "nCbuf", m_nCbuf);
		jKv(j, "bAutoBound", m_autoBound);
		jKv(j, "bShowGrid", m_showGrid);
		jKv<float>(j, "vBgCol", m_background);
		IF_Le_F(m_port < 1 || m_port > 65535 || m_maxClients < 1 || m_maxClients > 64, "Invalid viewer port/client limit");
		IF_Le_F(m_nPbuf <= 0 || m_nLbuf <= 0 || m_nCbuf < 0, "Invalid geometry buffer limits");
		// Resolve relative to launch directory, then the executable's copied assets.
		if (!std::filesystem::is_directory(m_root) && !j.contains("webRoot"))
		{
			std::error_code ec;
			auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
			if (!ec)
				m_root = (exe.parent_path() / "html/webViewer3D").string();
		}
		IF_Le_F(!std::filesystem::is_regular_file(std::filesystem::path(m_root) / "vendor/three.module.min.js"),
				"Viewer assets not found in webRoot: " + m_root);
		return true;
	}
	bool _WebViewer3D::link(const json &j, ModuleMgr *manager)
	{
		IF_F(!manager || !_GeometryBase::link(j, manager));
		m_objects.clear();
		m_vpGb.clear();
		std::vector<std::string> sources;
		jKv(j, "vGeometryBase", sources);
		for (const auto &name : sources)
		{
			auto source = dynamic_cast<_GeometryBase *>(static_cast<BASE *>(manager->findModule(name)));
			IF_Le_F(!source, "Geometry source not found: " + name);
			if (std::find(m_vpGb.begin(), m_vpGb.end(), source) != m_vpGb.end())
				continue;
			m_vpGb.push_back(source);
			Object o;
			o.source = source;
			o.name = source->getName();
			o.nP = m_nPbuf;
			o.nL = m_nLbuf;
			o.nC = m_nCbuf;
			m_objects.push_back(o);
		}
		// Optional per-source style/caps. The base's vGeometryBase name list still works.
		const auto &objects = jK(j, "vGeometry");
		if (objects.is_array())
			for (const auto &config : objects)
			{
				std::string name;
				IF_Le_F(!jKv(config, "_GeometryBase", name), "vGeometry entry needs _GeometryBase");
				auto source = dynamic_cast<_GeometryBase *>(static_cast<BASE *>(manager->findModule(name)));
				IF_Le_F(!source, "Geometry source not found: " + name);
				auto it = std::find_if(m_objects.begin(), m_objects.end(), [source](const Object &o)
									   { return o.source == source; });
				if (it == m_objects.end())
				{
					Object o;
					o.source = source;
					o.name = name;
					o.nP = m_nPbuf;
					o.nL = m_nLbuf;
					o.nC = m_nCbuf;
					m_objects.push_back(o);
					it = m_objects.end() - 1;
					m_vpGb.push_back(source);
				}
				jKv(config, "bVisible", it->visible);
				jKv(config, "nP", it->nP);
				jKv(config, "nL", it->nL);
				jKv(config, "nC", it->nC);
				jKv(config, "matPointSize", it->pointSize);
				jKv<float>(config, "matCol", it->color);
				IF_Le_F(it->nP < 0 || it->nL < 0 || it->nC < 0 || !std::isfinite(it->pointSize) || it->pointSize <= 0, "Invalid geometry style/limits");
				it->nP = std::min(it->nP, m_nPbuf);
				it->nL = std::min(it->nL, m_nLbuf);
				it->nC = std::min(it->nC, m_nCbuf);
			}
		// Verify the worst case once, before any threads start.
		uint64_t bytes = webviewer3d::HeaderBytes;
		for (const auto &o : m_objects)
			if (o.visible)
			{
				bytes += webviewer3d::ObjectBytes + uint64_t(o.nP) * 16 + uint64_t(o.nL) * 32;
				if (dynamic_cast<_OctreeGrid *>(o.source))
					bytes += webviewer3d::GridHeaderBytes + uint64_t(o.nC) * webviewer3d::CellBytes + 3;
			}
		IF_Le_F(m_objects.size() > 1024 || bytes > webviewer3d::MaxFrameBytes, "Reduce geometry caps: frame limit is 64 MiB / 1024 objects");
		return true;
	}
	std::string _WebViewer3D::hello() const
	{
		json j;
		j["type"] = "hello";
		j["version"] = webviewer3d::Version;
		j["autoBound"] = m_autoBound;
		j["showGrid"] = m_showGrid;
		j["background"] = {m_background.x, m_background.y, m_background.z};
		j["camera"] = {
			{"eye", {m_camPose.m_vEye.x, m_camPose.m_vEye.y, m_camPose.m_vEye.z}},
			{"target", {m_camPose.m_vLookAt.x, m_camPose.m_vLookAt.y, m_camPose.m_vLookAt.z}},
			{"up", {m_camPose.m_vUp.x, m_camPose.m_vUp.y, m_camPose.m_vUp.z}},
			{"type", m_camProj.m_type},
			{"fov", m_camProj.m_fov},
			{"near", std::max(0.001f, m_camProj.m_vNF.x)},
			{"far", m_camProj.m_vNF.y == FLT_MAX ? 1000000.f : m_camProj.m_vNF.y},
			{"lr", {m_camProj.m_vLR.x, m_camProj.m_vLR.y}},
			{"bt", {m_camProj.m_vBT.x, m_camProj.m_vBT.y}}};
		j["objects"] = json::array();
		for (size_t i = 0; i < m_objects.size(); ++i)
			j["objects"].push_back({{"id", i}, {"name", m_objects[i].name}});
		return j.dump();
	}
	bool _WebViewer3D::start()
	{
		IF_F(m_running || !m_pT);
		m_stream.reset(new WebSocketStream(m_http->context(), hello(), m_maxClients));
		std::string error;
		IF_Le_F(!m_http->start(m_host, uint16_t(m_port), m_root, m_stream->upgradeHandler(), &error), error);
		m_running = true;
		m_pT->run();
		try
		{
			// An owned/joined worker avoids _Thread's asynchronous cancellation during teardown.
			m_worker = std::thread([this]
								   {
				while (m_running)
				{
					auto deadline = std::chrono::steady_clock::now() +
						std::chrono::microseconds(int64_t(1000000 / m_pT->getTargetFPS()));
					if (!m_paused && m_stream->nClient()) updateAllGeometries();
					std::unique_lock<std::mutex> lock(m_waitMutex);
					m_wakeup.wait_until(lock, deadline, [this] { return !m_running; });
				} });
		}
		catch (const std::exception &e)
		{
			stop();
			LOG_E(e.what());
			return false;
		}
		LOG_I("WebViewer3D: http://" + m_host + ":" + i2str(m_port) + "/");
		return true;
	}
	void _WebViewer3D::stop()
	{
		m_running = false;
		m_wakeup.notify_all();
		if (m_worker.joinable())
			m_worker.join();
		m_http->stop();
		if (m_stream)
			m_stream->stop();
		if (m_pT)
			m_pT->stop();
	}
	void _WebViewer3D::pause() { m_paused = true; }
	void _WebViewer3D::resume() { m_paused = false; }
	void _WebViewer3D::updateAllGeometries()
	{
		std::shared_ptr<std::vector<uint8_t>> frame;
		for (auto &buffer : m_buffers)
			if (buffer.use_count() == 1)
			{
				frame = buffer;
				break;
			}
		if (!frame)
		{
			frame = std::make_shared<std::vector<uint8_t>>();
			m_buffers.push_back(frame); // bounded by connected clients plus producer/latest
		}
		const uint64_t now = getApproxTbootUs();
		const uint64_t expiry = m_dTexpire && now > m_dTexpire ? now - m_dTexpire : 0;
		webviewer3d::begin(*frame, ++m_sequence, now);
		uint32_t count = 0;
		for (size_t i = 0; i < m_objects.size(); ++i)
			if (m_objects[i].visible)
			{
				collect(m_objects[i], *frame, uint32_t(i), expiry);
				++count;
			}
		webviewer3d::finish(*frame, count);
		m_frameBytes = frame->size();
		m_stream->publish(frame);
	}
	void _WebViewer3D::collect(const Object &o, std::vector<uint8_t> &frame, uint32_t id, uint64_t expiry)
	{
		m_points.clear();
		m_lines.clear();
		m_pointColors.clear();
		m_lineColors.clear();
		float bounds[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
		auto finite = [](const vFloat3 &p)
		{ return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z); };
		auto colorByte = [](float c)
		{ return uint8_t(std::clamp(std::isfinite(c) ? c : 1.f, 0.f, 1.f) * 255.f + 0.5f); };
		auto vertex = [&](const vFloat3 &p, vFloat3 c, std::vector<float> &positions, std::vector<uint8_t> &colors)
		{
			positions.insert(positions.end(), {p.x, p.y, p.z});
			if (c.x <= 0 && c.y <= 0 && c.z <= 0)
				c = vFloat3(o.color.x, o.color.y, o.color.z);
			colors.insert(colors.end(), {colorByte(c.x), colorByte(c.y), colorByte(c.z), 255});
			bounds[0] = std::min(bounds[0], p.x);
			bounds[1] = std::min(bounds[1], p.y);
			bounds[2] = std::min(bounds[2], p.z);
			bounds[3] = std::max(bounds[3], p.x);
			bounds[4] = std::max(bounds[4], p.y);
			bounds[5] = std::max(bounds[5], p.z);
		};
		// Match ImGUIviewer: reset insertion index, respect returned count and buffer capacity.
		m_grPt.m_iT = 0;
		int n = o.nP ? std::min(o.source->get(&m_grPt, expiry), m_grPt.m_nT) : 0;
		for (int i = 0; i < n && m_points.size() / 3 < size_t(o.nP); ++i)
		{
			const auto &p = *m_grPt.get(i);
			if (p.m_tStamp && p.m_tStamp >= expiry && finite(p.m_vP))
				vertex(p.m_vP, p.m_vC, m_points, m_pointColors);
		}
		m_grLn.m_iT = 0;
		n = o.nL ? std::min(o.source->get(&m_grLn, expiry), m_grLn.m_nT) : 0;
		for (int i = 0; i < n && m_lines.size() / 6 < size_t(o.nL); ++i)
		{
			const auto &l = *m_grLn.get(i);
			if (!l.m_tStamp || l.m_tStamp < expiry || !finite(l.m_vPa) || !finite(l.m_vPb))
				continue;
			vertex(l.m_vPa, l.m_vC, m_lines, m_lineColors);
			vertex(l.m_vPb, l.m_vC, m_lines, m_lineColors);
		}
		OCTGRID_CELLS *cells = nullptr;
		if (auto *grid = dynamic_cast<_OctreeGrid *>(o.source))
		{
			grid->get(&m_cells, expiry, size_t(o.nC));
			cells = &m_cells;
			if (!m_cells.m_vCell.empty())
				for (size_t axis = 0; axis < 3; ++axis)
				{
					const float c = m_cells.m_header.m_vPorigin[axis], h = m_cells.m_header.m_vRootCellSize[axis] * 0.5f;
					bounds[axis] = std::min(bounds[axis], c - h);
					bounds[axis + 3] = std::max(bounds[axis + 3], c + h);
				}
		}
		if (m_points.empty() && m_lines.empty() && (!cells || cells->m_vCell.empty()))
			std::fill(bounds, bounds + 6, 0.f);
		webviewer3d::object(frame, id, o.pointSize, colorByte(o.color.w) / 255.f, bounds, m_points, m_pointColors, m_lines, m_lineColors, cells);
	}
	void _WebViewer3D::console(void *console)
	{
		_GeometryViewerBase::console(console);
		if (console)
			static_cast<_Console *>(console)->addMsg("Web clients: " + std::to_string(m_stream ? m_stream->nClient() : 0) + ", frame bytes: " + std::to_string(m_frameBytes.load()));
	}
}
