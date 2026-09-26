#include "_WebSelectableOctGrid.h"
#include "../../../Module/ModuleMgr.h"
#include "../../../IO/WebSocketStream.h"
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace kai
{
	_WebSelectableOctGrid::_WebSelectableOctGrid() : m_http(new HttpServer)
	{ for (size_t i = 0; i < m_streams.size(); ++i) m_streams[i].type = webselectableoctgrid::Types[i]; }
	_WebSelectableOctGrid::~_WebSelectableOctGrid() { stop(); }

	bool _WebSelectableOctGrid::loadConfig(void)
	{
		IF_F(!_GeometryViewerBase::loadConfig());
		const json &j = *m_pJ;
		jKv(j, "host", m_host);
		jKv(j, "port", m_port);
		jKv(j, "webRoot", m_root);
		jKv(j, "nClientMax", m_maxClients);
		jKv(j, "nCbuf", m_nCbuf);
		jKv(j, "bAutoBound", m_autoBound);
		jKv(j, "bShowGrid", m_showGrid);
		jKv<float>(j, "vBgCol", m_background);
		IF_Le_F(m_port < 1 || m_port > 65535 || m_maxClients < 1 || m_maxClients > 64, "Invalid viewer port/client limit");
		IF_Le_F(m_nPbuf < 0 || m_nLbuf < 0 || m_nCbuf < 0, "Invalid geometry buffer limits");
		// Resolve relative to launch directory, then the executable's copied assets.
		if (!std::filesystem::is_directory(m_root) && !j.contains("webRoot"))
		{
			std::error_code ec;
			auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
			if (!ec)
				m_root = (exe.parent_path() / "html/viewer/_SelectableOctGrid").string();
		}
		IF_Le_F(!std::filesystem::is_regular_file(std::filesystem::path(m_root) / "vendor/three.module.min.js"),
				"Viewer assets not found in webRoot: " + m_root);
		return true;
	}

	bool _WebSelectableOctGrid::saveConfig(bool bExport)
	{
		if (!_GeometryViewerBase::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		j["host"] = m_host;
		j["port"] = m_port;
		j["webRoot"] = m_root;
		j["nClientMax"] = m_maxClients;
		j["nCbuf"] = m_nCbuf;
		j["bAutoBound"] = m_autoBound;
		j["bShowGrid"] = m_showGrid;
		j["vBgCol"] = {m_background.x(), m_background.y(), m_background.z(), m_background.w()};

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _WebSelectableOctGrid::link(void)
	{
		IF_F(!_GeometryViewerBase::link());
		const json &j = *m_pJ;
		string error;
		IF_Le_F(!m_sources.link(j, m_pM, m_nPbuf, m_nLbuf, m_nCbuf, error), error);
		IF_Le_F(m_sources.m_vGeometry.size() + m_sources.m_vGrid.size() > 1024, "Viewer source limit is 1024");
		for (auto type : webselectableoctgrid::Types)
		{
			uint64_t bytes = webselectableoctgrid::HeaderBytes;
			if (type == webselectableoctgrid::Type::Cells)
			{
				for (const auto &source : m_sources.m_vGrid) if (source.m_bVisible)
					bytes += webselectableoctgrid::ObjectBytes + webselectableoctgrid::GridHeaderBytes +
						uint64_t(source.m_nC) * webselectableoctgrid::CellBytes;
			}
			else
			{
				for (const auto &source : m_sources.m_vGeometry) if (includes(source, type))
					bytes += webselectableoctgrid::ObjectBytes + webselectableoctgrid::vertexBytes(
						type == webselectableoctgrid::Type::Points ? size_t(source.m_nP) : size_t(source.m_nL) * 2);
			}
			IF_Le_F(bytes > webselectableoctgrid::MaxFrameBytes, string(webselectableoctgrid::name(type)) + " stream exceeds 64 MiB; reduce its caps");
		}
		return true;
	}
	bool _WebSelectableOctGrid::includes(const VIEWER_GEOMETRY_SOURCE &source, webselectableoctgrid::Type type) const
	{
		return source.m_bVisible && (type == webselectableoctgrid::Type::Points ? source.m_nP > 0 : source.m_nL > 0);
	}
	std::string _WebSelectableOctGrid::hello(webselectableoctgrid::Type type) const
	{
		json j;
		j["type"] = "hello";
		j["version"] = webselectableoctgrid::Version;
		j["stream"] = webselectableoctgrid::name(type);
		j["autoBound"] = m_autoBound;
		j["showGrid"] = m_showGrid;
		j["background"] = {m_background.x(), m_background.y(), m_background.z()};
		j["camera"] = {
			{"eye", {m_camPose.m_vEye.x(), m_camPose.m_vEye.y(), m_camPose.m_vEye.z()}},
			{"target", {m_camPose.m_vLookAt.x(), m_camPose.m_vLookAt.y(), m_camPose.m_vLookAt.z()}},
			{"up", {m_camPose.m_vUp.x(), m_camPose.m_vUp.y(), m_camPose.m_vUp.z()}},
			{"type", m_camProj.m_type},
			{"fov", m_camProj.m_fov},
			{"near", std::max(0.001f, m_camProj.m_vNF.x())},
			{"far", m_camProj.m_vNF.y() == FLT_MAX ? 1000000.f : m_camProj.m_vNF.y()},
			{"lr", {m_camProj.m_vLR.x(), m_camProj.m_vLR.y()}},
			{"bt", {m_camProj.m_vBT.x(), m_camProj.m_vBT.y()}}};
		j["objects"] = json::array();
		uint32_t id = 0;
		for (const auto &source : m_sources.m_vGeometry)
			j["objects"].push_back({{"id", id++}, {"name", source.m_name}, {"selectableGrid", false}});
		for (const auto &source : m_sources.m_vGrid)
			j["objects"].push_back({{"id", id++}, {"name", source.m_name}, {"selectableGrid", true}});
		return j.dump();
	}
	bool _WebSelectableOctGrid::start()
	{
		IF_F(m_running || !m_pT);
		std::vector<std::pair<std::string, WebSocketStream *>> routes;
		for (auto &stream : m_streams)
		{
			stream.transport.reset(new WebSocketStream(m_http->context(), hello(stream.type), m_maxClients));
			routes.emplace_back(string("/stream/") + webselectableoctgrid::name(stream.type), stream.transport.get());
		}
		std::string error;
		IF_Le_F(!m_http->start(m_host, uint16_t(m_port), m_root, WebSocketStream::routes(routes), &error), error);
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
					if (!m_paused) updateAllGeometries();
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
		LOG_I("WebSelectableOctGrid: http://" + m_host + ":" + i2str(m_port) + "/");
		return true;
	}
	void _WebSelectableOctGrid::stop()
	{
		m_running = false;
		m_wakeup.notify_all();
		if (m_worker.joinable())
			m_worker.join();
		m_http->stop();
		for (auto &stream : m_streams) if (stream.transport) stream.transport->stop();
		if (m_pT)
			m_pT->stop();
	}
	void _WebSelectableOctGrid::pause() { m_paused = true; }
	void _WebSelectableOctGrid::resume() { m_paused = false; }
	void _WebSelectableOctGrid::updateAllGeometries()
	{
		for (auto &stream : m_streams) if (stream.transport->nClient())
		{
			try { publish(stream); }
			catch (const std::exception &e) { LOG_E(string(webselectableoctgrid::name(stream.type)) + " stream: " + e.what()); }
		}
	}
	void _WebSelectableOctGrid::publish(Stream &stream)
	{
		std::shared_ptr<std::vector<uint8_t>> frame;
		for (auto &buffer : stream.buffers) if (buffer.use_count() == 1) { frame = buffer; break; }
		if (!frame)
		{
			frame = std::make_shared<std::vector<uint8_t>>();
			stream.buffers.push_back(frame);
		}
		const uint64_t now = getTns();
		const uint64_t expiry = m_dTexpire && now > m_dTexpire ? now - m_dTexpire : 0;
		webselectableoctgrid::begin(*frame, stream.type, ++stream.sequence, now);
		uint32_t count = 0;
		if (stream.type == webselectableoctgrid::Type::Cells)
		{
			for (size_t i = 0; i < m_sources.m_vGrid.size(); ++i)
			{
				const auto &source = m_sources.m_vGrid[i];
				if (!source.m_bVisible) continue;
				collectCells(source, *frame, uint32_t(m_sources.m_vGeometry.size() + i), expiry);
				++count;
			}
		}
		else
		{
			for (size_t i = 0; i < m_sources.m_vGeometry.size(); ++i)
			{
				const auto &source = m_sources.m_vGeometry[i];
				if (!includes(source, stream.type)) continue;
				collectGeometry(source, stream.type, *frame, uint32_t(i), expiry);
				++count;
			}
		}
		webselectableoctgrid::finish(*frame, count);
		stream.bytes = frame->size();
		stream.transport->publish(frame);
	}
	void _WebSelectableOctGrid::collectGeometry(const VIEWER_GEOMETRY_SOURCE &o, webselectableoctgrid::Type type, std::vector<uint8_t> &frame, uint32_t id, uint64_t expiry)
	{
		using webselectableoctgrid::Type;
		float bounds[6] = {FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};
		auto colorByte = [](float c) { return uint8_t(std::clamp(std::isfinite(c) ? c : 1.f, 0.f, 1.f) * 255.f + .5f); };
		m_positions.clear(); m_colors.clear();
		auto finite = [](const Vector3f &p) { return std::isfinite(p.x()) && std::isfinite(p.y()) && std::isfinite(p.z()); };
		auto vertex = [&](const Vector3f &p, Vector3f c) {
			m_positions.insert(m_positions.end(), {p.x(), p.y(), p.z()});
			if (c.x() <= 0 && c.y() <= 0 && c.z() <= 0) c = o.m_matCol.head<3>();
			m_colors.insert(m_colors.end(), {colorByte(c.x()), colorByte(c.y()), colorByte(c.z())});
			bounds[0] = std::min(bounds[0], p.x()); bounds[1] = std::min(bounds[1], p.y()); bounds[2] = std::min(bounds[2], p.z());
			bounds[3] = std::max(bounds[3], p.x()); bounds[4] = std::max(bounds[4], p.y()); bounds[5] = std::max(bounds[5], p.z());
		};
		if (type == Type::Points)
		{
			m_grPt.m_iT = 0;
			const int count = std::min(o.m_pGeometry->get(&m_grPt, expiry), m_grPt.m_nT);
			for (int i = 0; i < count && m_positions.size() / 3 < size_t(o.m_nP); ++i)
			{
				const auto &p = *m_grPt.get(i);
				if (p.m_tStamp && p.m_tStamp >= expiry && finite(p.m_vP)) vertex(p.m_vP, p.m_vC);
			}
		}
		else
		{
			m_grLn.m_iT = 0;
			const int count = std::min(o.m_pGeometry->get(&m_grLn, expiry), m_grLn.m_nT);
			for (int i = 0; i < count && m_positions.size() / 6 < size_t(o.m_nL); ++i)
			{
				const auto &l = *m_grLn.get(i);
				if (!l.m_tStamp || l.m_tStamp < expiry || !finite(l.m_vPa) || !finite(l.m_vPb)) continue;
				vertex(l.m_vPa, l.m_vC); vertex(l.m_vPb, l.m_vC);
			}
		}
		if (m_positions.empty()) std::fill(bounds, bounds + 6, 0.f);
		if (type == Type::Points) webselectableoctgrid::points(frame, id, o.m_matPointSize, bounds, m_positions, m_colors);
		else webselectableoctgrid::lines(frame, id, bounds, m_positions, m_colors);
	}
	void _WebSelectableOctGrid::collectCells(const VIEWER_GRID_SOURCE &source, std::vector<uint8_t> &frame, uint32_t id, uint64_t expiry)
	{
		source.m_pGrid->get(&m_cells, expiry, size_t(source.m_nC));
		float bounds[6];
		for (size_t axis = 0; axis < 3; ++axis)
		{
			const float center = m_cells.m_header.m_vPorigin[axis], half = m_cells.m_header.m_vRootCellSize[axis] * .5f;
			bounds[axis] = center - half; bounds[axis + 3] = center + half;
		}
		const float opacity = std::clamp(source.m_matCol.w(), 0.f, 1.f);
		webselectableoctgrid::cells(frame, id, opacity, bounds, m_cells);
	}
	void _WebSelectableOctGrid::console(void *console)
	{
		_GeometryViewerBase::console(console);
		if (console) for (const auto &stream : m_streams)
			static_cast<_Console *>(console)->addMsg(string(webselectableoctgrid::name(stream.type)) + " clients: " +
				std::to_string(stream.transport ? stream.transport->nClient() : 0) + ", frame bytes: " + std::to_string(stream.bytes.load()));
	}
}
