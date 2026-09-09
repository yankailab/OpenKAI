#include "WebSocketStream.h"
#include <boost/asio/post.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <atomic>
#include <mutex>
#include <unordered_map>

namespace kai
{
	namespace net = boost::asio;
	namespace beast = boost::beast;
	namespace ws = beast::websocket;

	struct WebSocketStream::State : std::enable_shared_from_this<State>
	{
		struct Session;
		net::io_context &io;
		const std::string hello;
		const size_t limit;
		std::atomic<size_t> clients{0};
		std::mutex mutex;
		Frame latest;
		uint64_t version = 0;
		bool notification = false;
		std::unordered_map<Session *, std::weak_ptr<Session>> sessions;

		State(net::io_context &i, std::string h, size_t n) : io(i), hello(std::move(h)), limit(n) {}
		struct Session : std::enable_shared_from_this<Session>
		{
			std::shared_ptr<State> state;
			ws::stream<beast::tcp_stream, false> socket;
			beast::flat_buffer input{4096};
			Frame sending;
			uint64_t sentVersion = 0;
			bool writing = true, enabled = false, awaitingAck = false, closed = false;

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
					self->socket.async_write(net::buffer(self->state->hello),
						[self](beast::error_code ec, size_t) {
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
			void send()
			{
				if (closed || writing || !enabled || awaitingAck) return;
				{
					std::lock_guard<std::mutex> lock(state->mutex);
					if (!state->latest || state->version == sentVersion) return;
					sending = state->latest;
					sentVersion = state->version;
				}
				writing = awaitingAck = true;
				socket.binary(true);
				socket.async_write(net::buffer(*sending), [self = shared_from_this()](beast::error_code ec, size_t) {
					self->sending.reset();
					self->writing = false;
					if (ec) return self->close();
					self->send(); // ACK may arrive before the write completion handler.
				});
			}
		};
	};

	WebSocketStream::WebSocketStream(net::io_context &io, std::string hello, size_t maxClients)
		: m_state(std::make_shared<State>(io, std::move(hello), maxClients)) {}
	WebSocketStream::~WebSocketStream() = default;
	HttpServer::Upgrade WebSocketStream::upgradeHandler()
	{
		return [state = m_state](beast::tcp_stream stream, HttpServer::Request request) {
			if (state->clients >= state->limit) return;
			auto session = std::make_shared<State::Session>(state, std::move(stream));
			state->sessions[session.get()] = session;
			session->accept(std::move(request));
		};
	}
	HttpServer::Upgrade WebSocketStream::routes(const std::vector<std::pair<std::string, WebSocketStream *>> &streams)
	{
		std::vector<std::pair<std::string, HttpServer::Upgrade>> handlers;
		for (const auto &entry : streams) handlers.emplace_back(entry.first, entry.second->upgradeHandler());
		return [handlers = std::move(handlers)](beast::tcp_stream stream, HttpServer::Request request) {
			for (const auto &entry : handlers)
				if (request.target() == entry.first) return entry.second(std::move(stream), std::move(request));
			auto socket = std::make_shared<beast::tcp_stream>(std::move(stream));
			auto response = std::make_shared<beast::http::response<beast::http::string_body>>(beast::http::status::not_found, request.version());
			response->body() = "Unknown geometry stream";
			response->prepare_payload();
			socket->expires_after(std::chrono::seconds(5));
			beast::http::async_write(*socket, *response, [socket, response](beast::error_code, size_t) {});
		};
	}
	void WebSocketStream::publish(Frame frame)
	{
		std::lock_guard<std::mutex> lock(m_state->mutex);
		m_state->latest = std::move(frame);
		++m_state->version;
		if (m_state->notification) return;
		m_state->notification = true;
		net::post(m_state->io, [state = m_state] {
			{
				std::lock_guard<std::mutex> lock(state->mutex);
				state->notification = false;
			}
			for (auto it = state->sessions.begin(); it != state->sessions.end();)
			{
				if (auto s = it->second.lock()) { ++it; s->send(); }
				else it = state->sessions.erase(it);
			}
		});
	}
	size_t WebSocketStream::nClient() const { return m_state->clients.load(); }
	void WebSocketStream::stop()
	{
		while (!m_state->sessions.empty())
		{
			auto it = m_state->sessions.begin();
			if (auto s = it->second.lock()) s->close();
			else m_state->sessions.erase(it);
		}
		std::lock_guard<std::mutex> lock(m_state->mutex);
		m_state->latest.reset();
	}
}
