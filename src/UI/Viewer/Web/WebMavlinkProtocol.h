#ifndef OpenKAI_src_UI_Viewer_Web_WebMavlinkProtocol_H_
#define OpenKAI_src_UI_Viewer_Web_WebMavlinkProtocol_H_

#include "../../../DataObject/MavlinkStream.h"

namespace kai::webmavlink
{
	inline constexpr const char *Protocol = "openkai.mavlink";
	inline constexpr int Version = 1;
	inline constexpr const char *Endpoint = "/stream/mavlink";

	// All numbers use SI units except degrees for geography and degrees heading.
	// Host receive timestamps determine freshness; missing messages are JSON null.
	json hello(const json &scene, uint64_t staleAfterMs);
	json telemetry(MavlinkStream &stream, uint64_t sequence, uint64_t nowNs,
		uint64_t unixTimeMs, uint64_t staleAfterMs);
}
#endif
