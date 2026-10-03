#include "_WebMavlinkStream.h"
#include "WebMavlinkProtocol.h"
#include "../../../IO/WebSocketStream.h"
#include "../../_Console.h"
#include <chrono>
#include <filesystem>

namespace kai
{
	_WebMavlinkStream::_WebMavlinkStream() : m_http(new HttpServer) {}
	_WebMavlinkStream::~_WebMavlinkStream() { stop(); }

	bool _WebMavlinkStream::loadConfig()
	{
		stop();
		IF_F(!_ModuleBase::loadConfig());
		const json &j = *m_pJ;
		jKv(j, "host", m_host);
		jKv(j, "port", m_port);
		jKv(j, "webRoot", m_root);
		jKv(j, "modelsRoot", m_modelsRoot);
		jKv(j, "MavlinkStream", m_streamName);
		jKv(j, "nClientMax", m_maxClients);
		jKv(j, "staleAfterMs", m_staleAfterMs);
		if (j.contains("scene")) m_scene = j["scene"];
		IF_Le_F(!m_scene.is_object(), "scene must be a JSON object");
		IF_Le_F(m_port < 1 || m_port > 65535 || m_maxClients < 1 || m_maxClients > 64, "Invalid viewer port/client limit");
		IF_Le_F(m_staleAfterMs < 100 || m_staleAfterMs > 3600000, "staleAfterMs must be 100..3600000");
		IF_Le_F(m_pT->getTargetFPS() > 100, "MAVLink viewer polling FPS must be 1..100");
		if (!std::filesystem::is_directory(m_root) && !j.contains("webRoot"))
		{
			std::error_code ec;
			auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
			if (!ec) m_root = (exe.parent_path() / "html/viewer/mavlink").string();
		}
		IF_Le_F(!std::filesystem::is_regular_file(std::filesystem::path(m_root) / "index.html"),
			"Viewer index.html not found in webRoot: " + m_root);
		return true;
	}

	bool _WebMavlinkStream::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		json &j = *m_pJ;
		j["host"] = m_host;
		j["port"] = m_port;
		j["webRoot"] = m_root;
		j["modelsRoot"] = m_modelsRoot;
		j["MavlinkStream"] = m_streamName;
		j["nClientMax"] = m_maxClients;
		j["staleAfterMs"] = m_staleAfterMs;
		j["scene"] = m_scene;
		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _WebMavlinkStream::link(InstanceMgr *pM)
	{
		IF_F(!_ModuleBase::link(pM));
		m_stream = dynamic_cast<MavlinkStream *>(static_cast<DataObjBase *>(pM->findDataObject(m_streamName)));
		IF_Le_F(!m_stream, "MavlinkStream not found: " + m_streamName);
		return true;
	}

	bool _WebMavlinkStream::check()
	{
		return m_stream && _ModuleBase::check();
	}

	bool _WebMavlinkStream::start()
	{
		IF_F(m_running || !check());
		m_paused = false;
		m_sequence = 0;
		// HttpServer owns a single-use io_context; destroy transport references first.
		m_transport.reset();
		m_http.reset(new HttpServer);
		m_transport.reset(new WebSocketStream(m_http->context(), webmavlink::hello(m_scene, m_staleAfterMs).dump(),
			m_maxClients, WebSocketStream::Mode::TextPush));
		HttpServer::Mounts mounts;
		if (!m_modelsRoot.empty() && std::filesystem::is_directory(m_modelsRoot))
			mounts.emplace_back("/models", m_modelsRoot);
		else
			LOG_I("MAVLink map/model directory unavailable: " + m_modelsRoot);
		std::string error;
		IF_Le_F(!m_http->start(m_host, uint16_t(m_port), m_root,
			WebSocketStream::routes({{webmavlink::Endpoint, m_transport.get()}}), &error, 32, mounts), error);
		m_running = true;
		m_pT->run();
		try
		{
			m_worker = std::thread([this] {
				while (m_running)
				{
					const auto deadline = std::chrono::steady_clock::now() +
						std::chrono::microseconds(1000000 / m_pT->getTargetFPS());
					if (!m_paused && m_transport->nClient())
					{
						try { publish(); }
						catch (const std::exception &e) { LOG_E(string("MAVLink viewer: ") + e.what()); }
					}
					std::unique_lock<std::mutex> lock(m_waitMutex);
					m_wakeup.wait_until(lock, deadline, [this] { return !m_running; });
				}
			});
		}
		catch (const std::exception &e)
		{
			stop();
			LOG_E(e.what());
			return false;
		}
		LOG_I("WebMavlinkStream: http://" + m_host + ":" + i2str(m_port) + "/");
		return true;
	}

	void _WebMavlinkStream::publish()
	{
		const auto unixTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		const auto payload = webmavlink::telemetry(*m_stream, ++m_sequence, getTns(), unixTimeMs, m_staleAfterMs)
			.dump(-1, ' ', false, json::error_handler_t::replace);
		auto frame = std::make_shared<std::vector<uint8_t>>(payload.begin(), payload.end());
		m_frameBytes = frame->size();
		m_transport->publish(frame);
	}

	void _WebMavlinkStream::stop()
	{
		m_running = false;
		m_wakeup.notify_all();
		if (m_worker.joinable()) m_worker.join();
		m_http->stop();
		if (m_transport) m_transport->stop();
		if (m_pT) m_pT->stop();
	}
	void _WebMavlinkStream::pause() { m_paused = true; }
	void _WebMavlinkStream::resume() { m_paused = false; }
	void _WebMavlinkStream::console(void *pConsole)
	{
		_ModuleBase::console(pConsole);
		if (pConsole) static_cast<_Console *>(pConsole)->addMsg("MAVLink clients: " +
			std::to_string(m_transport ? m_transport->nClient() : 0) + ", frame bytes: " + std::to_string(m_frameBytes.load()));
	}
}
