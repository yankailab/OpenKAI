#include "_HttpServer.h"
#include "HttpServer.h"
#include <filesystem>

namespace kai
{
	_HttpServer::_HttpServer() = default;
	_HttpServer::~_HttpServer() { stop(); }

	bool _HttpServer::loadConfig()
	{
		stop();
		IF_F(!_ModuleBase::loadConfig());
		const json &j = *m_pJ;
		jKv(j, "host", m_host);
		jKv(j, "port", m_port);
		jKv(j, "webRoot", m_root);
		jKv(j, "nClientMax", m_maxClients);
		IF_Le_F(m_port < 1 || m_port > 65535 || m_maxClients < 1 || m_maxClients > 64,
			"Invalid HTTP port/client limit");
		std::error_code error;
		IF_Le_F(!std::filesystem::is_directory(m_root, error), "HTTP webRoot not found: " + m_root);
		return true;
	}

	bool _HttpServer::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		json &j = *m_pJ;
		j["host"] = m_host;
		j["port"] = m_port;
		j["webRoot"] = m_root;
		j["nClientMax"] = m_maxClients;
		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _HttpServer::start()
	{
		IF_F(m_http || !check());
		// HttpServer is single-use; create a fresh worker after each stop.
		auto server = std::make_unique<HttpServer>();
		std::string error;
		IF_Le_F(!server->start(m_host, uint16_t(m_port), m_root, {}, &error, m_maxClients), error);
		m_http = std::move(server);
		LOG_I("HttpServer: http://" + m_host + ":" + i2str(m_port) + "/");
		return true;
	}

	void _HttpServer::stop()
	{
		if (m_http) m_http->stop();
		m_http.reset();
		_ModuleBase::stop();
	}

	bool _HttpServer::bRun() { return bool(m_http); }
	bool _HttpServer::bRunning() { return bool(m_http); }
	bool _HttpServer::bStopped() { return !m_http; }
}
