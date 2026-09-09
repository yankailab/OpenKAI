#include "../../src/3D/Viewer/WebViewer3DProtocol.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main()
{
	using namespace kai;
	OCTGRID_CELLS grid;
	grid.m_header = {{10, -20, 30}, {8, 4, 2}, 40, 123456789};
	for (unsigned octant = 0; octant < 8; ++octant)
	{
		UUID128 path(0);
		for (unsigned depth = 0; depth <= 40; ++depth)
		{
			auto id = path;
			id |= uint64_t(depth);
			OCTGRID_CELL cell;
			cell.setID(id);
			cell.m_vC[0] = 17; cell.m_vC[1] = 128; cell.m_vC[2] = 255;
			assert(cell.id().m_uint64[0] == id.m_uint64[0]);
			assert(cell.id().m_uint64[1] == id.m_uint64[1]);
			std::array<float, 3> c, s;
			assert(octgridCellBox(grid.m_header, cell.id(), c, s));
			for (unsigned axis = 0; axis < 3; ++axis)
			{
				const double scale = std::ldexp(1.0, -int(depth));
				const double sign = octant & (4 >> axis) ? 1 : -1;
				assert(s[axis] == float(grid.m_header.m_vRootCellSize[axis] * scale));
				assert(c[axis] == float(grid.m_header.m_vPorigin[axis] + sign * grid.m_header.m_vRootCellSize[axis] * (1 - scale) / 2));
			}
			if (depth < 40)
			{
				auto invalid = id;
				invalid |= uint64_t(64); // unused path bit
				assert(!octgridCellBox(grid.m_header, invalid, c, s));
				path |= UUID128(octant) << (123 - depth * 3);
			}
			grid.m_vCell.assign(depth % 4, cell); // every possible alignment remainder
			std::vector<uint8_t> frame;
			float bounds[6] = {};
			webviewer3d::begin(frame, 1, 2);
			webviewer3d::object(frame, 0, 2, 1, bounds, {}, {}, {}, {}, &grid);
			const size_t expected = (32 + 64 + 40 + 19 * grid.m_vCell.size() + 3) & ~size_t(3);
			assert(frame.size() == expected);
			assert(frame[44] == grid.m_vCell.size() && frame[80] == 1);
			if (!grid.m_vCell.empty())
			{
				assert(std::memcmp(frame.data() + 136, cell.m_ID, 16) == 0);
				assert(frame[152] == 17 && frame[153] == 128 && frame[154] == 255);
			}
			webviewer3d::object(frame, 1, 2, 1, bounds, {1, 2, 3}, {1, 2, 3, 255}, {}, {});
			assert(frame.size() == expected + 80 && frame[expected] == 1);
		}
	}
	std::cout << "PASS: 19-byte records, depths 0-40 in all octants, canonical IDs, colors and mixed-object alignment\n";
}
