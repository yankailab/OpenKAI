#ifndef OpenKAI_src_3D_Viewer_WebViewer3DProtocol_H_
#define OpenKAI_src_3D_Viewer_WebViewer3DProtocol_H_

#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace kai::webviewer3d
{
	constexpr uint32_t Magic = 0x31443357; // W3D1
	constexpr uint32_t Version = 1;
	constexpr size_t HeaderBytes = 32, ObjectBytes = 64;
	constexpr size_t MaxFrameBytes = 64 * 1024 * 1024;

	inline void u32(std::vector<uint8_t> &b, size_t at, uint32_t v)
	{
		for (unsigned i = 0; i < 4; ++i) b.at(at + i) = uint8_t(v >> (8 * i));
	}
	inline void f32(std::vector<uint8_t> &b, size_t at, float v)
	{
		static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559, "IEEE float32 required");
		uint32_t bits;
		std::memcpy(&bits, &v, 4);
		u32(b, at, bits);
	}
	inline void floats(std::vector<uint8_t> &b, const std::vector<float> &v)
	{
		if (v.empty()) return;
		const size_t at = b.size();
		b.resize(at + v.size() * 4);
		const uint16_t endian = 1;
		if (*reinterpret_cast<const uint8_t *>(&endian)) std::memcpy(b.data() + at, v.data(), v.size() * 4);
		else for (size_t i = 0; i < v.size(); ++i) f32(b, at + i * 4, v[i]);
	}
	inline void begin(std::vector<uint8_t> &b, uint32_t sequence, uint64_t timestampUs)
	{
		b.assign(HeaderBytes, 0);
		u32(b, 0, Magic); u32(b, 4, Version); u32(b, 8, sequence);
		u32(b, 24, uint32_t(timestampUs)); u32(b, 28, uint32_t(timestampUs >> 32));
	}
	inline void object(std::vector<uint8_t> &b, uint32_t id, float pointSize, float opacity,
		const float bounds[6], const std::vector<float> &points, const std::vector<uint8_t> &pointColors,
		const std::vector<float> &lines, const std::vector<uint8_t> &lineColors)
	{
		const size_t nP = points.size() / 3, nL = lines.size() / 6;
		if (points.size() % 3 || lines.size() % 6 || pointColors.size() != nP * 4 || lineColors.size() != nL * 8)
			throw std::invalid_argument("Invalid geometry attribute lengths");
		if (b.size() + ObjectBytes + nP * 16 + nL * 32 > MaxFrameBytes)
			throw std::length_error("Geometry exceeds the 64 MiB frame limit");
		const size_t at = b.size();
		b.resize(at + ObjectBytes, 0);
		u32(b, at, id); u32(b, at + 4, uint32_t(nP)); u32(b, at + 8, uint32_t(nL));
		f32(b, at + 16, pointSize); f32(b, at + 20, opacity);
		for (size_t i = 0; i < 6; ++i) f32(b, at + 24 + i * 4, bounds[i]);
		floats(b, points); b.insert(b.end(), pointColors.begin(), pointColors.end());
		floats(b, lines); b.insert(b.end(), lineColors.begin(), lineColors.end());
	}
	inline void finish(std::vector<uint8_t> &b, uint32_t objects)
	{
		u32(b, 12, objects); u32(b, 16, uint32_t(b.size()));
	}
}
#endif
