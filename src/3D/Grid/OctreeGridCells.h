#ifndef OpenKAI_src_3D_Grid_OctreeGridCells_H_
#define OpenKAI_src_3D_Grid_OctreeGridCells_H_

#include "../../Primitive/UUID128.h"
#include <array>
#include <cstddef>
#include <vector>

namespace kai
{
	constexpr int OCTGRID_MAX_LEVEL = 40;

	// Byte arrays avoid UUID alignment padding. IDs are always little-endian,
	// low word first, both in memory and on the wire.
	struct OCTGRID_CELL
	{
		uint8_t m_ID[16] = {};
		uint8_t m_vC[4] = {0, 0, 0, 255}; // RGBA8

		void setID(const UUID128 &id)
		{
			for (unsigned i = 0; i < 16; ++i)
				m_ID[i] = uint8_t(id.m_uint64[i / 8] >> (8 * (i % 8)));
		}
		UUID128 id() const
		{
			UUID128 result(0);
			for (unsigned i = 0; i < 16; ++i)
				result.m_uint64[i / 8] |= uint64_t(m_ID[i]) << (8 * (i % 8));
			return result;
		}
	};
	static_assert(sizeof(OCTGRID_CELL) == 20, "Grid cells must occupy exactly 20 bytes");

	struct OCTGRID_HEADER
	{
		std::array<float, 3> m_vPorigin = {}; // root center, not its minimum corner
		std::array<float, 3> m_vRootCellSize = {1, 1, 1}; // full XYZ extents
		uint32_t m_nMaxLevel = 0;
		uint64_t m_tStamp = 0; // publication time, shared by all records
	};

	struct OCTGRID_CELLS
	{
		OCTGRID_HEADER m_header;
		std::vector<OCTGRID_CELL> m_vCell; // complete snapshot, including occupied ancestors
	};

	// Decode without converting the 128-bit ID to a floating-point number.
	// Level 0 is the root; depth child segments start at bit 123, then 120, ...
	inline bool octgridCellBox(const OCTGRID_HEADER &header, const UUID128 &id,
		std::array<float, 3> &center, std::array<float, 3> &size)
	{
		unsigned depth = id.m_uint64[0] & 63;
		if (header.m_nMaxLevel > OCTGRID_MAX_LEVEL || depth > header.m_nMaxLevel || id.m_uint64[1] >> 62)
			return false;
		uint64_t high = id.m_uint64[1], low = id.m_uint64[0] & ~uint64_t(63);
		std::array<double, 3> c = {header.m_vPorigin[0], header.m_vPorigin[1], header.m_vPorigin[2]};
		std::array<double, 3> s = {header.m_vRootCellSize[0], header.m_vRootCellSize[1], header.m_vRootCellSize[2]};
		for (unsigned level = 0; level < depth; ++level)
		{
			const unsigned child = (high >> 59) & 7;
			for (unsigned axis = 0; axis < 3; ++axis)
			{
				c[axis] += s[axis] * ((child & (4 >> axis)) ? 0.25 : -0.25);
				s[axis] *= 0.5;
			}
			high = (high << 3) | (low >> 61);
			low <<= 3;
		}
		// Consumed path bits may remain in the two reserved high bits.
		if ((high & (UINT64_MAX >> 2)) || low) return false;
		for (unsigned axis = 0; axis < 3; ++axis)
		{
			center[axis] = float(c[axis]);
			size[axis] = float(s[axis]);
		}
		return true;
	}
}
#endif
