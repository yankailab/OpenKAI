#ifndef OpenKAI_src_IO_WebSocketStream_H_
#define OpenKAI_src_IO_WebSocketStream_H_

#include "../Net/HttpServer.h"
#include <cstdint>
#include <vector>

namespace kai
{
	// Shared immutable snapshots, one write in flight per browser. Geometry streams
	// additionally wait for an application ACK; TextPush needs no client messages.
	// publish() is thread safe and coalesces notifications; socket IO stays on
	// HttpServer's worker.
	class WebSocketStream
	{
	public:
		enum class Mode { BinaryAcknowledged, TextPush };
		using Frame = std::shared_ptr<const std::vector<uint8_t>>;
		WebSocketStream(boost::asio::io_context &io, std::string hello, size_t maxClients = 8,
			Mode mode = Mode::BinaryAcknowledged);
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
