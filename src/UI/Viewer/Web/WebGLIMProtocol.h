#ifndef OpenKAI_src_UI_Viewer_Web_WebGLIMProtocol_H_
#define OpenKAI_src_UI_Viewer_Web_WebGLIMProtocol_H_

#include "../../../Net/HttpServer.h"
#include "../../../DataObject/PCLmap.h"
#include <cstdint>
#include <memory>

namespace kai
{
	namespace webglim
	{
		// Dedicated accumulating-submap protocol, unrelated to geometry snapshots.
		constexpr uint32_t Magic = 0x334d4c47; // LE bytes "GLM3"
		constexpr uint32_t Version = 3;
		constexpr uint32_t HeaderBytes = 56;
		constexpr uint32_t MaxChunkPoints = 65536;
		constexpr uint32_t MaxSubmapPoints = 10000000;
		// Binary chunk header, all little-endian:
		// u32 magic,version,kind=1,headerBytes; u64 session,id,timestampNs;
		// u32 totalPoints,offsetPoints,countPoints,reserved=0; float32 xyz[].
		// Text data: reset(session,mapTimestampNs), submap(session,mapTimestampNs,id,
		// timestampNs,pointCount,pose), pose(session,mapTimestampNs,id,pose).
		// All u64 JSON fields are decimal strings; pose is column-major 4x4.
		// Client start/pause/next grants one outstanding data message. Every
		// text data message and binary chunk requires next; hello does not.
		class Stream
		{
		public:
			Stream(boost::asio::io_context &io, std::string hello, size_t maxClients = 8);
			~Stream();
			HttpServer::Upgrade upgradeHandler();
			// Retain the complete copied map for reconnects and slow clients.
			void publish(std::vector<PCLmap::Submap> submaps, uint64_t session, uint64_t timestamp);
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
