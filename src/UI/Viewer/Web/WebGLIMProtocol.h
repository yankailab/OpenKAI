#ifndef OpenKAI_src_UI_Viewer_Web_WebGLIMProtocol_H_
#define OpenKAI_src_UI_Viewer_Web_WebGLIMProtocol_H_

#include "../../../Net/HttpServer.h"
#include <cstdint>
#include <memory>

namespace kai
{
	struct GLIM_MAP_SNAPSHOT;

	namespace webglim
	{
		// Dedicated accumulating-submap protocol, unrelated to geometry snapshots.
		constexpr uint32_t Magic = 0x324d4c47; // LE bytes "GLM2"
		constexpr uint32_t Version = 2;
		constexpr uint32_t HeaderBytes = 56;
		constexpr uint32_t MaxChunkPoints = 65536;
		constexpr uint32_t MaxSubmapPoints = 10000000;
		// Binary chunk header, all little-endian:
		// u32 magic,version,kind=1,headerBytes; u64 session,id,timestampNs;
		// u32 totalPoints,offsetPoints,countPoints,reserved=0; float32 xyz[].
		// Text data: reset(session,revision), submap(session,revision,id,
		// timestampNs,pointCount,pose), pose(session,revision,id,pose).
		// All u64 JSON fields are decimal strings; pose is column-major 4x4.
		// Client start/pause/next grants one outstanding data message. Every
		// text data message and binary chunk requires next; hello does not.
		class Stream
		{
		public:
			Stream(boost::asio::io_context &io, std::string hello, size_t maxClients = 8);
			~Stream();
			HttpServer::Upgrade upgradeHandler();
			// Full descriptor set for a revision; unchanged point arrays are shared.
			// Coalescing revisions cannot drop previously completed submaps.
			void publish(GLIM_MAP_SNAPSHOT snapshot);
			size_t nClient() const;
			// Stop producer and HttpServer before calling stop().
			void stop();
		private:
			struct State;
			std::shared_ptr<State> m_state;
		};
	}
}
#endif
