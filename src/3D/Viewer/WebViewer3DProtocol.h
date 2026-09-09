#ifndef OpenKAI_src_3D_Viewer_WebViewer3DProtocol_H_
#define OpenKAI_src_3D_Viewer_WebViewer3DProtocol_H_

#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>
#include "../Grid/OctreeGridCells.h"

namespace kai::webviewer3d
{
	constexpr uint32_t Magic = 0x31443357; // W3D1
	constexpr uint32_t Version = 2;
	constexpr size_t HeaderBytes = 32, ObjectBytes = 64;
	constexpr size_t GridHeaderBytes = 40, CellBytes = 19;
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
	inline void u64(std::vector<uint8_t> &b, size_t at, uint64_t v)
	{
		u32(b, at, uint32_t(v)); u32(b, at + 4, uint32_t(v >> 32));
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
		const std::vector<float> &lines, const std::vector<uint8_t> &lineColors,
		const OCTGRID_CELLS *grid = nullptr)
	{
		const size_t nP = points.size() / 3, nL = lines.size() / 6;
		if (points.size() % 3 || lines.size() % 6 || pointColors.size() != nP * 4 || lineColors.size() != nL * 8)
			throw std::invalid_argument("Invalid geometry attribute lengths");
		const size_t gridBytes = grid ? GridHeaderBytes + grid->m_vCell.size() * CellBytes : 0;
		const size_t end = (b.size() + ObjectBytes + nP * 16 + nL * 32 + gridBytes + 3) & ~size_t(3);
		if (end > MaxFrameBytes)
			throw std::length_error("Geometry exceeds the 64 MiB frame limit");
		const size_t at = b.size();
		b.resize(at + ObjectBytes, 0);
		u32(b, at, id); u32(b, at + 4, uint32_t(nP)); u32(b, at + 8, uint32_t(nL));
		f32(b, at + 16, pointSize); f32(b, at + 20, opacity);
		for (size_t i = 0; i < 6; ++i) f32(b, at + 24 + i * 4, bounds[i]);
		floats(b, points); b.insert(b.end(), pointColors.begin(), pointColors.end());
		floats(b, lines); b.insert(b.end(), lineColors.begin(), lineColors.end());
		if (grid)
		{
			u32(b, at + 12, uint32_t(grid->m_vCell.size()));
			u32(b, at + 48, 1); // grid header present even for an empty snapshot
			const size_t start = b.size();
			b.resize(start + GridHeaderBytes, 0);
			for (size_t i = 0; i < 3; ++i)
			{
				f32(b, start + i * 4, grid->m_header.m_vPorigin[i]);
				f32(b, start + 12 + i * 4, grid->m_header.m_vRootCellSize[i]);
			}
			u32(b, start + 24, grid->m_header.m_nMaxLevel);
			u64(b, start + 32, grid->m_header.m_tStamp);
			if (!grid->m_vCell.empty())
			{
				const auto *bytes = reinterpret_cast<const uint8_t *>(grid->m_vCell.data());
				b.insert(b.end(), bytes, bytes + grid->m_vCell.size() * CellBytes);
			}
			b.resize(end, 0); // align the next object's float arrays, not each cell
		}
	}
	inline void finish(std::vector<uint8_t> &b, uint32_t objects)
	{
		u32(b, 12, objects); u32(b, 16, uint32_t(b.size()));
	}
}
#endif
