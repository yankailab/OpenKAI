#ifndef OpenKAI_src_Net_HttpServer_H_
#define OpenKAI_src_Net_HttpServer_H_

// Framework consumers include ncurses, whose timeout() macro collides with Asio.
#pragma push_macro("timeout")
#undef timeout
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#pragma pop_macro("timeout")
#include <functional>
#include <memory>
#include <string>

namespace kai
{
	// Small asynchronous static-file server. No framework or geometry dependency.
	// Configure/start/stop from the owning thread; stop joins the IO worker.
	class HttpServer
	{
	public:
		using Request = boost::beast::http::request<boost::beast::http::string_body>;
		using Upgrade = std::function<void(boost::beast::tcp_stream, Request)>;
		HttpServer();
		~HttpServer();
		HttpServer(const HttpServer &) = delete;
		HttpServer &operator=(const HttpServer &) = delete;

		boost::asio::io_context &context();
		bool start(const std::string &host, uint16_t port, const std::string &root,
				   Upgrade upgrade, std::string *error = nullptr, size_t maxConnections = 32);
		void stop();
		uint16_t port() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};
}
#endif
