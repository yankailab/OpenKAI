#include "HttpServer.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <filesystem>
#include <thread>

namespace kai
{
	namespace net = boost::asio;
	namespace beast = boost::beast;
	namespace http = beast::http;
	namespace fs = std::filesystem;
	using tcp = net::ip::tcp;

	static std::string mimeType(const fs::path &p)
	{
		const auto ext = p.extension().string();
		if (ext == ".html") return "text/html; charset=utf-8";
		if (ext == ".js" || ext == ".mjs") return "text/javascript; charset=utf-8";
		if (ext == ".css") return "text/css; charset=utf-8";
		if (ext == ".json") return "application/json";
		if (ext == ".svg") return "image/svg+xml";
		if (ext == ".png") return "image/png";
		if (ext == ".ico") return "image/x-icon";
		return "application/octet-stream";
	}

	struct HttpServer::Impl
	{
		net::io_context io{1};
		tcp::acceptor acceptor{io};
		std::thread worker;
		fs::path root;
		Upgrade upgrade;
		size_t active = 0, limit = 32;
		uint16_t boundPort = 0;
		bool started = false;

		struct Session : std::enable_shared_from_this<Session>
		{
			Impl &server;
			beast::tcp_stream stream;
			beast::flat_buffer buffer{16384};
			http::request_parser<http::string_body> parser;

			Session(Impl &s, tcp::socket socket) : server(s), stream(std::move(socket))
			{
				++server.active;
				parser.header_limit(8192);
				parser.body_limit(0);
			}
			~Session() { --server.active; }
			void run()
			{
				stream.expires_after(std::chrono::seconds(10));
				http::async_read(stream, buffer, parser,
					[self = shared_from_this()](beast::error_code ec, size_t) {
						if (!ec) self->respond();
					});
			}
			template<class Body> void send(http::response<Body> response)
			{
				response.set(http::field::server, "OpenKAI");
				response.set("X-Content-Type-Options", "nosniff");
				response.set(http::field::cache_control, "no-cache");
				response.keep_alive(false);
				auto message = std::make_shared<http::response<Body>>(std::move(response));
				stream.expires_after(std::chrono::seconds(15));
				http::async_write(stream, *message,
					[self = shared_from_this(), message](beast::error_code, size_t) {
						beast::error_code ignored;
						self->stream.socket().shutdown(tcp::socket::shutdown_send, ignored);
					});
			}
			void fail(http::status status)
			{
				http::response<http::string_body> r{status, 11};
				r.set(http::field::content_type, "text/plain; charset=utf-8");
				r.body() = std::string(http::obsolete_reason(status));
				r.prepare_payload();
				send(std::move(r));
			}
			void respond()
			{
				auto req = parser.release();
				if (beast::websocket::is_upgrade(req) && server.upgrade)
				{
					stream.expires_never();
					server.upgrade(std::move(stream), std::move(req));
					return;
				}
				if (req.method() != http::verb::get && req.method() != http::verb::head)
					return fail(http::status::method_not_allowed);
				std::string target(req.target());
				target = target.substr(0, target.find('?'));
				// Decode before checking containment, including encoded traversal/separators.
				std::string decoded;
				for (size_t i = 0; i < target.size(); ++i)
				{
					unsigned char c = target[i];
					if (c == '%')
					{
						auto hex = [](char h) -> int {
							if (h >= '0' && h <= '9') return h - '0';
							if (h >= 'a' && h <= 'f') return h - 'a' + 10;
							if (h >= 'A' && h <= 'F') return h - 'A' + 10;
							return -1;
						};
						if (i + 2 >= target.size() || hex(target[i+1]) < 0 || hex(target[i+2]) < 0)
							return fail(http::status::bad_request);
						c = hex(target[i+1]) * 16 + hex(target[i+2]);
						i += 2;
					}
					if (c < 32 || c == 127 || c == '\\' || c == ':') return fail(http::status::bad_request);
					decoded += char(c);
				}
				if (decoded.empty() || decoded.front() != '/') return fail(http::status::bad_request);
				if (decoded == "/") decoded = "/index.html";
				std::error_code ec;
				const auto path = fs::canonical(server.root / decoded.substr(1), ec);
				if (ec) return fail(http::status::not_found);
				auto p = path.begin();
				for (auto r = server.root.begin(); r != server.root.end(); ++r, ++p)
					if (p == path.end() || *p != *r) return fail(http::status::forbidden);
				if (!fs::is_regular_file(path, ec) || ec) return fail(http::status::not_found);
				beast::error_code fileError;
				http::file_body::value_type body;
				body.open(path.string().c_str(), beast::file_mode::scan, fileError);
				if (fileError) return fail(http::status::not_found);
				if (req.method() == http::verb::head)
				{
					http::response<http::empty_body> r{http::status::ok, 11};
					r.set(http::field::content_type, mimeType(path));
					r.content_length(body.size());
					return send(std::move(r));
				}
				http::response<http::file_body> r{http::status::ok, 11};
				r.set(http::field::content_type, mimeType(path));
				r.content_length(body.size());
				r.body() = std::move(body);
				send(std::move(r));
			}
		};

		void accept()
		{
			acceptor.async_accept([this](beast::error_code ec, tcp::socket socket) {
				if (!ec && active < limit)
				{
					beast::error_code ignored;
					socket.set_option(tcp::no_delay(true), ignored);
					std::make_shared<Session>(*this, std::move(socket))->run();
				}
				if (acceptor.is_open()) accept();
			});
		}
	};

	HttpServer::HttpServer() : m_impl(new Impl) {}
	HttpServer::~HttpServer() { stop(); }
	net::io_context &HttpServer::context() { return m_impl->io; }
	uint16_t HttpServer::port() const { return m_impl->boundPort; }
	bool HttpServer::start(const std::string &host, uint16_t port, const std::string &root,
		Upgrade upgrade, std::string *error, size_t maxConnections)
	{
		try
		{
			if (m_impl->started) throw std::runtime_error("HttpServer already started; create a new instance to restart");
			m_impl->root = fs::canonical(root);
			if (!fs::is_directory(m_impl->root)) throw std::runtime_error("Invalid HTTP root");
			tcp::resolver resolver(m_impl->io);
			auto endpoint = resolver.resolve(host, std::to_string(port), tcp::resolver::passive).begin()->endpoint();
			m_impl->acceptor.open(endpoint.protocol());
			m_impl->acceptor.set_option(tcp::acceptor::reuse_address(true));
			m_impl->acceptor.bind(endpoint);
			m_impl->acceptor.listen();
			m_impl->boundPort = m_impl->acceptor.local_endpoint().port();
			m_impl->limit = maxConnections;
			m_impl->upgrade = std::move(upgrade);
			m_impl->accept();
			m_impl->worker = std::thread([this] { m_impl->io.run(); });
			m_impl->started = true;
			return true;
		}
		catch (const std::exception &e)
		{
			if (error) *error = e.what();
			beast::error_code ignored;
			m_impl->acceptor.close(ignored);
			return false;
		}
	}
	void HttpServer::stop()
	{
		m_impl->io.stop();
		if (m_impl->worker.joinable()) m_impl->worker.join();
		beast::error_code ignored;
		m_impl->acceptor.close(ignored);
	}
}
