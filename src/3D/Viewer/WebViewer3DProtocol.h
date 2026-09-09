#ifndef OpenKAI_src_3D_Viewer_WebViewer3DProtocol_H_
#define OpenKAI_src_3D_Viewer_WebViewer3DProtocol_H_

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>
#include "../Grid/OctreeGridCells.h"

namespace kai::webviewer3d
{
	constexpr uint32_t Magic = 0x34443357; // W3D4; typed streams only
	constexpr uint32_t Version = 4;
	enum class Type : uint32_t { Points = 1, Lines = 2, Cells = 3 };
	constexpr std::array<Type, 3> Types = {Type::Points, Type::Lines, Type::Cells};
	inline const char *name(Type type)
	{
		switch (type) { case Type::Points: return "points"; case Type::Lines: return "lines"; case Type::Cells: return "cells"; }
		throw std::invalid_argument("Invalid geometry stream type");
	}
	constexpr size_t HeaderBytes = 32, ObjectBytes = 40, GridHeaderBytes = 40;
	constexpr size_t CellBytes = sizeof(OCTGRID_CELL), MaxFrameBytes = 64 * 1024 * 1024;

	inline void u32(std::vector<uint8_t> &b, size_t at, uint32_t v)
	{ for (unsigned i = 0; i < 4; ++i) b.at(at + i) = uint8_t(v >> (8 * i)); }
	inline void f32(std::vector<uint8_t> &b, size_t at, float v)
	{
		static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559, "IEEE float32 required");
		uint32_t bits; std::memcpy(&bits, &v, 4); u32(b, at, bits);
	}
	inline void u64(std::vector<uint8_t> &b, size_t at, uint64_t v)
	{ u32(b, at, uint32_t(v)); u32(b, at + 4, uint32_t(v >> 32)); }
	inline void floats(std::vector<uint8_t> &b, const std::vector<float> &v)
	{
		if (v.empty()) return;
		const size_t at = b.size(); b.resize(at + v.size() * 4);
		const uint16_t endian = 1;
		if (*reinterpret_cast<const uint8_t *>(&endian)) std::memcpy(b.data() + at, v.data(), v.size() * 4);
		else for (size_t i = 0; i < v.size(); ++i) f32(b, at + i * 4, v[i]);
	}
	inline void begin(std::vector<uint8_t> &b, Type type, uint32_t sequence, uint64_t timestampUs)
	{
		name(type); // validate before writing
		b.assign(HeaderBytes, 0);
		u32(b, 0, Magic); u32(b, 4, Version); u32(b, 8, uint32_t(type)); u32(b, 12, sequence);
		u64(b, 24, timestampUs);
	}
	inline void objectHeader(std::vector<uint8_t> &b, Type type, uint32_t id, size_t count,
		float pointSize, float opacity, const float bounds[6], size_t payloadBytes)
	{
		if (b.size() < HeaderBytes || b[8] != uint32_t(type)) throw std::invalid_argument("Geometry type does not match stream");
		if (count > UINT32_MAX || payloadBytes > MaxFrameBytes || b.size() + ObjectBytes + payloadBytes > MaxFrameBytes)
			throw std::length_error("Geometry stream exceeds the 64 MiB frame limit");
		const size_t at = b.size(); b.resize(at + ObjectBytes);
		u32(b, at, id); u32(b, at + 4, uint32_t(count));
		f32(b, at + 8, pointSize); f32(b, at + 12, opacity);
		for (size_t i = 0; i < 6; ++i) f32(b, at + 16 + i * 4, bounds[i]);
	}
	inline void vertices(std::vector<uint8_t> &b, Type type, uint32_t id, float pointSize, float opacity,
		const float bounds[6], const std::vector<float> &positions, const std::vector<uint8_t> &colors)
	{
		const size_t components = type == Type::Points ? 3 : 6;
		if (positions.size() % components || colors.size() != positions.size() / 3 * 4)
			throw std::invalid_argument("Invalid geometry attribute lengths");
		objectHeader(b, type, id, positions.size() / components, pointSize, opacity, bounds, positions.size() * 4 + colors.size());
		floats(b, positions); b.insert(b.end(), colors.begin(), colors.end());
	}
	inline void points(std::vector<uint8_t> &b, uint32_t id, float pointSize, float opacity,
		const float bounds[6], const std::vector<float> &positions, const std::vector<uint8_t> &colors)
	{ vertices(b, Type::Points, id, pointSize, opacity, bounds, positions, colors); }
	inline void lines(std::vector<uint8_t> &b, uint32_t id, float opacity,
		const float bounds[6], const std::vector<float> &positions, const std::vector<uint8_t> &colors)
	{ vertices(b, Type::Lines, id, 1, opacity, bounds, positions, colors); }
	inline void cells(std::vector<uint8_t> &b, uint32_t id, float opacity, const float bounds[6], const OCTGRID_CELLS &grid)
	{
		objectHeader(b, Type::Cells, id, grid.m_vCell.size(), 1, opacity, bounds, GridHeaderBytes + grid.m_vCell.size() * CellBytes);
		const size_t at = b.size(); b.resize(at + GridHeaderBytes, 0);
		for (size_t i = 0; i < 3; ++i)
		{
			f32(b, at + i * 4, grid.m_header.m_vPorigin[i]);
			f32(b, at + 12 + i * 4, grid.m_header.m_vRootCellSize[i]);
		}
		u32(b, at + 24, grid.m_header.m_nMaxLevel); u64(b, at + 32, grid.m_header.m_tStamp);
		if (!grid.m_vCell.empty())
		{
			const auto *data = reinterpret_cast<const uint8_t *>(grid.m_vCell.data());
			b.insert(b.end(), data, data + grid.m_vCell.size() * CellBytes);
		}
	}
	inline void finish(std::vector<uint8_t> &b, uint32_t objects)
	{ u32(b, 16, objects); u32(b, 20, uint32_t(b.size())); }
}
#endif
