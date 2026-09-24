#include "WebGLIMProtocol.h"
#include <boost/asio/post.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include "_WebGLIM.h"
#include "../../../Module/ModuleMgr.h"
#include "../../../SLAM/_GLIM.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

namespace kai
{
	namespace net = boost::asio;
	namespace beast = boost::beast;
	namespace ws = beast::websocket;

	namespace webglim
	{
		struct Stream::State : std::enable_shared_from_this<State>
		{
			struct Session;
			net::io_context &io;
			const std::string hello;
			const size_t limit;
			std::atomic<size_t> clients{0};
			std::mutex mutex;
			std::shared_ptr<const GLIM_MAP_SNAPSHOT> latest;
			bool notification = false;
			std::unordered_map<Session *, std::weak_ptr<Session>> sessions;
			State(net::io_context &i, std::string h, size_t n) : io(i), hello(std::move(h)), limit(n) {}

			struct Session : std::enable_shared_from_this<Session>
			{
				struct Delivered
				{
					bool announced = false;
					size_t points = 0;
					std::array<double, 16> pose{};
				};
				std::shared_ptr<State> state;
				ws::stream<beast::tcp_stream, false> socket;
				beast::flat_buffer input{4096};
				std::vector<uint8_t> sending;
				std::unordered_map<uint64_t, Delivered> delivered;
				uint64_t session = 0, revision = 0;
				size_t cursor = 0;
				bool haveSession = false, writing = true, enabled = false, awaitingAck = false, closed = false;

				Session(std::shared_ptr<State> s, beast::tcp_stream stream)
					: state(std::move(s)), socket(std::move(stream)) { ++state->clients; }
				~Session() { --state->clients; }
				void close()
				{
					if (closed) return;
					closed = true;
					state->sessions.erase(this);
					beast::error_code ignored;
					beast::get_lowest_layer(socket).socket().close(ignored);
				}
				void accept(HttpServer::Request request)
				{
					socket.set_option(ws::stream_base::timeout{std::chrono::seconds(10), std::chrono::seconds(30), true});
					socket.read_message_max(4096);
					socket.write_buffer_bytes(65536);
					socket.async_accept(request, [self = shared_from_this()](beast::error_code ec) {
						if (ec) return self->close();
						self->socket.text(true);
						self->socket.async_write(net::buffer(self->state->hello), [self](beast::error_code ec, size_t) {
							if (ec) return self->close();
							self->writing = false;
							self->send();
						});
						self->read();
					});
				}
				void read()
				{
					socket.async_read(input, [self = shared_from_this()](beast::error_code ec, size_t) {
						if (ec) return self->close();
						const auto cmd = beast::buffers_to_string(self->input.data());
						self->input.consume(self->input.size());
						if (!self->socket.got_text()) return self->close();
						if (cmd == "start") self->enabled = true;
						else if (cmd == "pause") self->enabled = false;
						else if (cmd == "next") self->awaitingAck = false;
						else return self->close();
						self->send();
						self->read();
					});
				}
				void write(bool binary)
				{
					writing = awaitingAck = true;
					socket.binary(binary);
					socket.async_write(net::buffer(sending), [self = shared_from_this()](beast::error_code ec, size_t) {
						self->writing = false;
						if (ec) return self->close();
						self->send();
					});
				}
				void text(const json &message)
				{
					const auto value = message.dump();
					sending.assign(value.begin(), value.end());
					write(false);
				}
				void u32(size_t at, uint32_t value)
				{
					for (size_t i = 0; i < 4; ++i) sending[at + i] = uint8_t(value >> (8 * i));
				}
				void u64(size_t at, uint64_t value)
				{
					for (size_t i = 0; i < 8; ++i) sending[at + i] = uint8_t(value >> (8 * i));
				}
				void send()
				{
					if (closed || writing || !enabled || awaitingAck) return;
					std::shared_ptr<const GLIM_MAP_SNAPSHOT> snapshot;
					{
						std::lock_guard<std::mutex> lock(state->mutex);
						snapshot = state->latest;
					}
					if (!snapshot) return;
					if (!haveSession || session != snapshot->session)
					{
						haveSession = true;
						session = snapshot->session;
						revision = snapshot->revision;
						cursor = 0;
						delivered.clear();
						return text({{"type", "reset"}, {"session", std::to_string(session)}, {"revision", std::to_string(revision)}});
					}
					if (revision != snapshot->revision)
					{
						revision = snapshot->revision;
						cursor = 0;
					}
					for (; cursor < snapshot->submaps.size(); ++cursor)
					{
						const auto &submap = snapshot->submaps[cursor];
						auto &sent = delivered[submap.id];
						const size_t total = submap.points ? submap.points->size() : 0;
						std::array<double, 16> pose;
						std::copy_n(submap.pose.data(), 16, pose.begin());
						if (!sent.announced)
						{
							sent.announced = true;
							sent.pose = pose;
							return text({{"type", "submap"}, {"session", std::to_string(session)}, {"revision", std::to_string(revision)},
								{"id", std::to_string(submap.id)}, {"timestampUs", std::to_string(submap.timestampUs)},
								{"pointCount", total}, {"pose", pose}});
						}
						if (sent.pose != pose)
						{
							sent.pose = pose;
							return text({{"type", "pose"}, {"session", std::to_string(session)}, {"revision", std::to_string(revision)},
								{"id", std::to_string(submap.id)}, {"pose", pose}});
						}
						if (sent.points < total)
						{
							const uint32_t count = uint32_t(std::min(total - sent.points, size_t(MaxChunkPoints)));
							sending.resize(HeaderBytes + size_t(count) * 12);
							u32(0, Magic); u32(4, Version); u32(8, 1); u32(12, HeaderBytes);
							u64(16, session); u64(24, submap.id); u64(32, submap.timestampUs);
							u32(40, uint32_t(total)); u32(44, uint32_t(sent.points)); u32(48, count); u32(52, 0);
							for (size_t i = 0; i < count; ++i)
							{
								const float *p = (*submap.points)[sent.points + i].data();
								for (size_t axis = 0; axis < 3; ++axis)
								{
									uint32_t bits;
									std::memcpy(&bits, p + axis, sizeof(bits));
									u32(HeaderBytes + i * 12 + axis * 4, bits);
								}
							}
							sent.points += count;
							return write(true);
						}
					}
				}
			};
		};

		Stream::Stream(net::io_context &io, std::string hello, size_t maxClients)
			: m_state(std::make_shared<State>(io, std::move(hello), maxClients)) {}
		Stream::~Stream() = default;
		HttpServer::Upgrade Stream::upgradeHandler()
		{
			return [state = m_state](beast::tcp_stream stream, HttpServer::Request request) {
				if (request.target() != "/stream/glim")
				{
					auto socket = std::make_shared<beast::tcp_stream>(std::move(stream));
					auto response = std::make_shared<beast::http::response<beast::http::string_body>>(beast::http::status::not_found, request.version());
					response->body() = "Unknown GLIM stream";
					response->prepare_payload();
					socket->expires_after(std::chrono::seconds(5));
					beast::http::async_write(*socket, *response, [socket, response](beast::error_code, size_t) {});
					return;
				}
				if (state->clients >= state->limit) return;
				auto session = std::make_shared<State::Session>(state, std::move(stream));
				state->sessions[session.get()] = session;
				session->accept(std::move(request));
			};
		}
		void Stream::publish(GLIM_MAP_SNAPSHOT snapshot)
		{
			std::unordered_set<uint64_t> ids;
			for (const auto &submap : snapshot.submaps)
			{
				if (!ids.insert(submap.id).second || !submap.pose.matrix().allFinite() ||
					(submap.points && submap.points->size() > MaxSubmapPoints))
					throw std::runtime_error("Invalid or oversized GLIM submap descriptor");
			}
			std::lock_guard<std::mutex> lock(m_state->mutex);
			m_state->latest = std::make_shared<GLIM_MAP_SNAPSHOT>(std::move(snapshot));
			if (m_state->notification) return;
			m_state->notification = true;
			net::post(m_state->io, [state = m_state] {
				{
					std::lock_guard<std::mutex> lock(state->mutex);
					state->notification = false;
				}
				for (auto it = state->sessions.begin(); it != state->sessions.end();)
				{
					if (auto session = it->second.lock()) { ++it; session->send(); }
					else it = state->sessions.erase(it);
				}
			});
		}
		size_t Stream::nClient() const { return m_state->clients.load(); }
		void Stream::stop()
		{
			while (!m_state->sessions.empty())
			{
				auto it = m_state->sessions.begin();
				if (auto session = it->second.lock()) session->close();
				else m_state->sessions.erase(it);
			}
			std::lock_guard<std::mutex> lock(m_state->mutex);
			m_state->latest.reset();
		}
	}

	_WebGLIM::_WebGLIM() = default;
	_WebGLIM::~_WebGLIM() { stop(); }
	bool _WebGLIM::init(const json &j)
	{
		// Keep common camera configuration without allocating geometry rings.
		json options = j;
		options["nPbuf"] = 0; options["nLbuf"] = 0;
		IF_F(!_GeometryViewerBase::init(options));
		jKv(j, "host", m_host); jKv(j, "port", m_port);
		jKv(j, "webRoot", m_root); jKv(j, "nClientMax", m_maxClients);
		jKv(j, "bAutoBound", m_autoBound); jKv(j, "bShowGrid", m_showGrid);
		jKv(j, "matPointSize", m_pointSize); jKv<float>(j, "vBgCol", m_background);
		IF_Le_F(m_port < 1 || m_port > 65535 || m_maxClients < 1 || m_maxClients > 64, "Invalid GLIM viewer port/client limit");
		IF_Le_F(!m_background.allFinite() || !std::isfinite(m_pointSize) || m_pointSize <= 0, "Invalid GLIM viewer style");
		if (!std::filesystem::is_directory(m_root) && !j.contains("webRoot"))
		{
			std::error_code ec;
			auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
			if (!ec) m_root = (exe.parent_path() / "html/viewer/_GLIM").string();
		}
		IF_Le_F(!std::filesystem::is_regular_file(std::filesystem::path(m_root) / "vendor/three.module.min.js"), "GLIM viewer assets not found: " + m_root);
		return true;
	}
	bool _WebGLIM::link(const json &j, ModuleMgr *manager)
	{
		IF_F(!_GeometryViewerBase::link(j, manager));
		string source;
		jKv(j, "_GLIM", source);
		m_slam = dynamic_cast<_GLIM *>(static_cast<BASE *>(manager->findModule(source)));
		IF_Le_F(!m_slam, "GLIM viewer source not found: " + source);
		return true;
	}
	std::string _WebGLIM::hello() const
	{
		return json{{"type", "hello"}, {"protocol", "openkai.glim"}, {"version", webglim::Version}, {"stream", "glim"},
			{"maxSubmapPoints", webglim::MaxSubmapPoints}, {"maxChunkPoints", webglim::MaxChunkPoints},
			{"autoBound", m_autoBound}, {"showGrid", m_showGrid}, {"pointSize", m_pointSize},
			{"background", {m_background.x(), m_background.y(), m_background.z()}},
			{"camera", {{"eye", {m_camPose.m_vEye.x(), m_camPose.m_vEye.y(), m_camPose.m_vEye.z()}},
				{"target", {m_camPose.m_vLookAt.x(), m_camPose.m_vLookAt.y(), m_camPose.m_vLookAt.z()}},
				{"up", {m_camPose.m_vUp.x(), m_camPose.m_vUp.y(), m_camPose.m_vUp.z()}},
				{"type", m_camProj.m_type}, {"fov", m_camProj.m_fov},
				{"near", std::max(0.001f, m_camProj.m_vNF.x())}, {"far", m_camProj.m_vNF.y() == FLT_MAX ? 1000000.f : m_camProj.m_vNF.y()},
				{"lr", {m_camProj.m_vLR.x(), m_camProj.m_vLR.y()}}, {"bt", {m_camProj.m_vBT.x(), m_camProj.m_vBT.y()}}}}}.dump();
	}
	bool _WebGLIM::start()
	{
		IF_F(m_running || !m_pT || !m_slam);
		m_http = std::make_unique<HttpServer>();
		m_stream = std::make_unique<webglim::Stream>(m_http->context(), hello(), m_maxClients);
		m_session = m_revision = UINT64_MAX;
		std::string error;
		IF_Le_F(!m_http->start(m_host, uint16_t(m_port), m_root, m_stream->upgradeHandler(), &error), error);
		m_running = true;
		m_pT->run();
		try
		{
			m_worker = std::thread([this] {
				while (m_running)
				{
					const auto deadline = std::chrono::steady_clock::now() + std::chrono::microseconds(int64_t(1000000 / m_pT->getTargetFPS()));
					if (!m_paused)
					{
						try { publish(); }
						catch (const std::exception &e) { LOG_E(string("GLIM stream: ") + e.what()); }
					}
					std::unique_lock<std::mutex> lock(m_waitMutex);
					m_wakeup.wait_until(lock, deadline, [this] { return !m_running; });
				}
			});
		}
		catch (const std::exception &e) { stop(); LOG_E(e.what()); return false; }
		LOG_I("WebGLIM: http://" + m_host + ":" + i2str(m_port) + "/");
		return true;
	}
	void _WebGLIM::publish()
	{
		auto snapshot = m_slam->submapSnapshot(m_session, m_revision);
		if (snapshot.session == m_session && snapshot.revision == m_revision) return;
		const auto session = snapshot.session, revision = snapshot.revision;
		m_stream->publish(std::move(snapshot));
		m_session = session; m_revision = revision;
	}
	void _WebGLIM::stop()
	{
		m_running = false;
		m_wakeup.notify_all();
		if (m_worker.joinable()) m_worker.join();
		if (m_http) m_http->stop();
		if (m_stream) m_stream->stop();
		// Destroy pending IO handlers while their state and HttpServer still exist.
		m_http.reset();
		m_stream.reset();
		if (m_pT) m_pT->stop();
	}
	void _WebGLIM::pause() { m_paused = true; }
	void _WebGLIM::resume() { m_paused = false; }
	void _WebGLIM::console(void *console)
	{
		_GeometryViewerBase::console(console);
	}
}
