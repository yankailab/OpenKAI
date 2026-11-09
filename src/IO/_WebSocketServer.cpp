/*
 * _WebSocketServer.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include "_WebSocketServer.h"
#include <future>
#include <unordered_map>

namespace kai
{
	namespace net = boost::asio;
	namespace beast = boost::beast;
	namespace websocket = beast::websocket;
	using tcp = net::ip::tcp;

	// All socket operations run on the read worker. The write worker waits for
	// completion without blocking receives, and can stop during a stalled send.
	struct _WebSocketServer::Backend
	{
		struct Session;
		_WebSocketServer &owner;
		net::io_context io{1};
		tcp::acceptor listener{io};
		net::steady_timer acceptRetry{io};
		std::unordered_map<uint64_t, std::shared_ptr<Session>> sessions;
		uint64_t nextClient = 0;

		explicit Backend(_WebSocketServer &server) : owner(server) {}

		struct Session : std::enable_shared_from_this<Session>
		{
			Backend &backend;
			const uint64_t id;
			websocket::stream<beast::tcp_stream, false> socket;
			beast::flat_buffer input;
			net::steady_timer writeTimeout;
			vector<uint8_t> output;
			std::shared_ptr<std::promise<bool>> pending;
			bool closed = false;
			bool waitingRead = false;

			Session(Backend &server, uint64_t client, tcp::socket stream)
				: backend(server), id(client), socket(std::move(stream)), writeTimeout(server.io) {}

			void close(void)
			{
				if (closed) return;
				closed = true;
				beast::error_code ignored;
				writeTimeout.cancel();
				socket.next_layer().socket().close(ignored);
				if (pending)
				{
					pending->set_value(false);
					pending.reset();
				}
				backend.owner.cbClose(id);
				backend.sessions.erase(id);
			}

			void accept(void)
			{
				socket.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
				socket.read_message_max(64 * 1024 * 1024);
				socket.async_accept([self = shared_from_this()](beast::error_code error) {
					if (error) return self->close();
					beast::error_code endpointError;
					const auto endpoint = self->socket.next_layer().socket().remote_endpoint(endpointError);
					if (endpointError) return self->close();
					self->backend.owner.cbOpen(self->id, endpoint.address().to_string(), std::to_string(endpoint.port()));
					self->read();
				});
			}

			void read(void)
			{
				socket.async_read(input, [self = shared_from_this()](beast::error_code error, size_t) {
					if (error) return self->close();
					self->deliver();
				});
			}

			void deliver(void)
			{
				if (closed) return;
				const auto bytes = input.data();
				if (!backend.owner.cbMessage(id, static_cast<const unsigned char *>(bytes.data()),
					bytes.size(), socket.got_text() ? 1 : 2))
				{
					// Keep one complete message per paused client without blocking the
					// event loop that owns accepts, write completions and timeouts.
					waitingRead = true;
					return;
				}
				waitingRead = false;
				input.consume(input.size());
				read();
			}

			void send(vector<uint8_t> bytes, bool binary, std::shared_ptr<std::promise<bool>> result)
			{
				if (closed || pending)
				{
					result->set_value(false);
					return;
				}
				output = std::move(bytes);
				pending = result;
				socket.binary(binary);
				if (backend.owner.m_tOutMs > 0)
				{
					writeTimeout.expires_after(std::chrono::milliseconds(backend.owner.m_tOutMs));
					writeTimeout.async_wait([self = shared_from_this(), result](beast::error_code error) {
						if (!error && self->pending == result) self->close();
					});
				}
				socket.async_write(net::buffer(output), [self = shared_from_this(), result](beast::error_code error, size_t) {
					self->writeTimeout.cancel();
					if (self->pending == result)
					{
						result->set_value(!error);
						self->pending.reset();
					}
					if (error) self->close();
				});
			}
		};

		bool open(string &error)
		{
			try
			{
				tcp::resolver resolver(io);
				const auto endpoints = resolver.resolve(owner.m_host, std::to_string(owner.m_port), tcp::resolver::passive);
				beast::error_code failure;
				for (const auto &entry : endpoints)
				{
					const auto endpoint = entry.endpoint();
					listener.open(endpoint.protocol(), failure);
					if (!failure) listener.set_option(tcp::acceptor::reuse_address(true), failure);
					if (!failure) listener.bind(endpoint, failure);
					if (!failure) listener.listen(net::socket_base::max_listen_connections, failure);
					if (!failure)
					{
						accept();
						return true;
					}
					beast::error_code ignored;
					listener.close(ignored);
				}
				error = failure.message();
			}
			catch (const std::exception &exception)
			{
				error = exception.what();
			}
			return false;
		}

		void accept(void)
		{
			listener.async_accept([this](beast::error_code error, tcp::socket socket) {
				if (!error && (owner.m_nClientMax < 0 || sessions.size() < static_cast<size_t>(owner.m_nClientMax)))
				{
					const uint64_t id = ++nextClient;
					auto session = std::make_shared<Session>(*this, id, std::move(socket));
					sessions.emplace(id, session);
					session->accept();
				}
				if (!listener.is_open()) return;
				if (error)
				{
					acceptRetry.expires_after(std::chrono::seconds(1));
					acceptRetry.async_wait([this](beast::error_code retryError) {
						if (!retryError && listener.is_open()) accept();
					});
				}
				else accept();
			});
		}

		void close(void)
		{
			beast::error_code ignored;
			acceptRetry.cancel();
			listener.close(ignored);
			while (!sessions.empty())
			{
				auto session = sessions.begin()->second;
				session->close();
			}
			io.stop();
		}
	};

	_WebSocketServer::_WebSocketServer()
	{
	}

	_WebSocketServer::~_WebSocketServer()
	{
		stop();
	}

	bool _WebSocketServer::loadConfig(void)
	{
		IF_F(!this->_IObase::loadConfig());
		json &j = *m_pJ;

		jKv(j, "wsMode", m_wsMode);
		jKv(j, "host", m_host);
		jKv(j, "port", m_port);
		jKv(j, "tOutMs", m_tOutMs);
		jKv(j, "nClientMax", m_nClientMax);
		jKv(j, "bReadAllClients", m_bReadAllClients);

		return true;
	}

	bool _WebSocketServer::saveConfig(bool bExport)
	{
		IF_F(!_IObase::saveConfig(false));

		json &j = *m_pJ;
		j["wsMode"] = m_wsMode;
		j["host"] = m_host;
		j["port"] = m_port;
		j["tOutMs"] = m_tOutMs;
		j["nClientMax"] = m_nClientMax;
		j["bReadAllClients"] = m_bReadAllClients;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _WebSocketServer::link(InstanceMgr *pM)
	{
		IF_F(!this->_IObase::link(pM));
		m_pM = pM;
		return true;
	}

	bool _WebSocketServer::start(void)
	{
		if (!m_pT || !m_pTr || !bStopped())
		{
			return false;
		}
		{
			std::lock_guard<std::mutex> lock(m_readMutex);
			m_bStopping = false;
			m_bReadPaused = false;
		}
		m_ioStatus = io_closed;
		if (!_IObase::start())
		{
			stop();
			return false;
		}
		return true;
	}

	void _WebSocketServer::pause(void)
	{
		{
			std::lock_guard<std::mutex> lock(m_readMutex);
			m_bReadPaused = true;
		}
		_IObase::pause();
	}

	void _WebSocketServer::resume(void)
	{
		{
			std::lock_guard<std::mutex> lock(m_readMutex);
			m_bReadPaused = false;
		}
		_IObase::resume();
		std::lock_guard<std::mutex> lock(m_backendMutex);
		if (m_backend)
		{
			net::post(m_backend->io, [backend = m_backend.get()] {
				for (const auto &entry : backend->sessions)
				{
					if (entry.second->waitingRead) entry.second->deliver();
				}
			});
		}
	}

	void _WebSocketServer::stop(void)
	{
		{
			std::lock_guard<std::mutex> lock(m_readMutex);
			m_bStopping = true;
		}
		_IObase::stop();
		m_ioStatus = io_closed;
	}

	void _WebSocketServer::updateW(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!m_pT->bRun())
			{
				break;
			}

			vector<std::shared_ptr<wsClient>> vClient;
			{
				std::lock_guard<std::mutex> lock(m_clientMutex);
				vClient = m_vClient;
			}
			if (m_pBpStreamIn && !vClient.empty())
			{
				vector<BYTE_PACKET> vBp;
				m_pBpStreamIn->get(vBp, m_tLastBpStreamIn);
				for (const BYTE_PACKET &bp : vBp)
				{
					if (!m_pT->bRun() || !sendPacket(vClient.front()->m_wsConn, bp))
					{
						break;
					}
					m_tLastBpStreamIn = bp.m_tStamp;
				}
			}

			for (const auto &pClient : vClient)
			{
				_WebSocket *pWS = pClient->m_pWS;
				if (!pWS->bOpen())
				{
					continue;
				}
				BytePacketStream *pStream = pWS->getBytePacketStreamIn();
				if (!pStream)
				{
					continue;
				}

				vector<BYTE_PACKET> vBp;
				pStream->get(vBp, pClient->m_tLastBpStreamIn);
				for (const BYTE_PACKET &bp : vBp)
				{
					if (!m_pT->bRun() || !sendPacket(pClient->m_wsConn, bp))
					{
						break;
					}
					pClient->m_tLastBpStreamIn = bp.m_tStamp;
				}
			}
		}
	}

	bool _WebSocketServer::sendClient(uint64_t client, const BYTE_PACKET &bp, bool binary)
	{
		std::shared_ptr<Backend> backend;
		{
			std::lock_guard<std::mutex> lock(m_backendMutex);
			backend = m_backend;
		}
		if (!backend) return false;
		auto result = std::make_shared<std::promise<bool>>();
		auto completed = result->get_future();
		net::post(backend->io, [pBackend = backend.get(), client, bytes = bp.m_vB, binary, result]() mutable {
			auto session = pBackend->sessions.find(client);
			if (session == pBackend->sessions.end()) result->set_value(false);
			else session->second->send(std::move(bytes), binary, result);
		});
		while (m_pT->bRun())
		{
			if (completed.wait_for(std::chrono::milliseconds(20)) == std::future_status::ready)
			{
				return completed.get();
			}
		}
		return false;
	}

	bool _WebSocketServer::sendPacket(uint64_t client, const BYTE_PACKET &bp)
	{
		if (m_wsMode == wsSocket_bin || m_wsMode == wsSocket_txt)
		{
			return sendClient(client, bp, m_wsMode == wsSocket_bin);
		}
		if (m_wsMode != wsSocket_bin_bcast && m_wsMode != wsSocket_txt_bcast)
		{
			return false;
		}

		vector<std::shared_ptr<wsClient>> vClient;
		{
			std::lock_guard<std::mutex> lock(m_clientMutex);
			vClient = m_vClient;
		}
		bool bSent = false;
		for (const auto &pClient : vClient)
		{
			if (!m_pT->bRun())
			{
				return false;
			}
			if (pClient->m_pWS->bOpen() && sendClient(pClient->m_wsConn, bp, m_wsMode == wsSocket_bin_bcast))
			{
				bSent = true;
			}
		}
		return bSent;
	}

	void _WebSocketServer::updateR(void)
	{
		while (m_pTr->bRun())
		{
			auto backend = std::make_shared<Backend>(*this);
			string error;
			if (!backend->open(error))
			{
				LOG_E("WebSocket listener: " + error);
				m_pTr->sleepT(NSEC_SEC);
				continue;
			}
			{
				std::lock_guard<std::mutex> lock(m_backendMutex);
				m_backend = backend;
			}
			m_ioStatus = io_opened;
			while (m_pTr->bRun() && !backend->io.stopped())
			{
				backend->io.run_for(std::chrono::milliseconds(20));
			}
			backend->close();
			{
				std::lock_guard<std::mutex> lock(m_backendMutex);
				m_backend.reset();
			}
			m_ioStatus = io_closed;
		}
	}

	int _WebSocketServer::nClient(void)
	{
		std::lock_guard<std::mutex> lock(m_clientMutex);
		return m_vClient.size();
	}

	_WebSocket *_WebSocketServer::getClient(int i)
	{
		std::lock_guard<std::mutex> lock(m_clientMutex);
		if (i < 0 || static_cast<size_t>(i) >= m_vClient.size())
		{
			return nullptr;
		}
		return m_vClient[i]->m_pWS;
	}

	void _WebSocketServer::cbOpen(uint64_t client, const string &addr, const string &port)
	{
		std::lock_guard<std::mutex> lock(m_clientMutex);
		if (m_nClientMax >= 0 && m_vClient.size() >= static_cast<size_t>(m_nClientMax))
		{
			return;
		}

		json j = json::object();
		j["name"] = this->getName() + ".WS" + i2str(m_vClient.size());
		j["class"] = "_WebSocket";
		j["thread"] = {{"FPS", 1}};

		auto pClient = std::make_shared<wsClient>();
		pClient->m_pJcfg = std::make_shared<JsonCfg>();
		pClient->m_pJcfg->setJson(j);
		pClient->m_pWS = new _WebSocket();
		_WebSocket *pWS = pClient->m_pWS;
		pWS->setName(j["name"].get<string>());
		pWS->setConfig(pClient->m_pJcfg.get(), pClient->m_pJcfg->getJson());
		if (!pWS->loadConfig() || !pWS->link(m_pM))
		{
			LOG_E("Accepted WebSocket configuration failed");
			return;
		}
		pWS->setIOstatus(io_opened);
		pClient->m_wsConn = client;
		m_vClient.push_back(pClient);

		LOG_I("Connection opened, addr: " + addr + ", port: " + port);
	}

	void _WebSocketServer::cbClose(uint64_t client)
	{
		std::lock_guard<std::mutex> lock(m_clientMutex);
		for (auto it = m_vClient.begin(); it != m_vClient.end(); ++it)
		{
			if ((*it)->m_wsConn == client)
			{
				(*it)->m_pWS->setIOstatus(io_closed);
				m_vClient.erase(it);
				LOG_I("Connection closed");
				return;
			}
		}
	}

	bool _WebSocketServer::cbMessage(uint64_t client,
									 const unsigned char *msg, uint64_t size, int type)
	{
		// Coordinate delivery with pause/stop. The IO worker preserves message
		// ordering and creates timestamps when messages enter the packet streams.
		std::unique_lock<std::mutex> readLock(m_readMutex);
		if (m_bStopping)
		{
			return true;
		}
		if (m_bReadPaused) return false;

		std::shared_ptr<wsClient> pClient;
		bool bDefaultClient = false;
		{
			std::lock_guard<std::mutex> lock(m_clientMutex);
			pClient = findWSclient(client);
			if (!pClient)
			{
				return true;
			}
			bDefaultClient = pClient == m_vClient.front();
		}

		vector<uint8_t> vB(msg, msg + size);
		BytePacketStream *pStream = pClient->m_pWS->getBytePacketStreamOut();
		if (pStream)
		{
			pStream->add({{vB, getTns()}});
		}
		// Default routing remains first-client-only; command servers can opt in
		// to receiving complete messages from every accepted connection.
		if (m_pBpStreamOut && (bDefaultClient || m_bReadAllClients))
		{
			m_pBpStreamOut->add({{vB, getTns()}});
		}
		readLock.unlock();

		LOG_I("Received message: " + string(reinterpret_cast<const char *>(msg), size) + ", size: " + i2str(size) + ", type: " + i2str(type));
		return true;
	}

	std::shared_ptr<wsClient> _WebSocketServer::findWSclient(uint64_t wsCli)
	{
		for (const auto &pClient : m_vClient)
		{
			if (pClient->m_wsConn == wsCli)
			{
				return pClient;
			}
		}
		return nullptr;
	}

	void _WebSocketServer::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);
		((_Console *)pConsole)->addMsg("nClients: " + i2str(nClient()), 1);
	}

}
