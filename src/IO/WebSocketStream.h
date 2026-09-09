#ifndef OpenKAI_src_IO_WebSocketStream_H_
#define OpenKAI_src_IO_WebSocketStream_H_

#include "../Net/HttpServer.h"
#include <cstdint>
#include <vector>

namespace kai
{
	// Shared immutable snapshots, one unacknowledged message per browser.
	// publish() is thread safe and coalesces notifications; socket IO stays on
	// HttpServer's worker. This deliberately bypasses IO_PACKET_FIFO.
	class WebSocketStream
	{
	public:
		using Frame = std::shared_ptr<const std::vector<uint8_t>>;
		WebSocketStream(boost::asio::io_context &io, std::string hello, size_t maxClients = 8);
		~WebSocketStream();
		HttpServer::Upgrade upgradeHandler();
		static HttpServer::Upgrade routes(const std::vector<std::pair<std::string, WebSocketStream *>> &streams);
		void publish(Frame frame);
		size_t nClient() const;
		// Call after HttpServer::stop(), with the producer joined.
		void stop();

	private:
		struct State;
		std::shared_ptr<State> m_state;
	};
}
#endif
