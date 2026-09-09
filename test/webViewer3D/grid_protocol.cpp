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
			cell.m_vC[3] = uint8_t(depth * 6);
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
			grid.m_vCell.assign(depth % 4, cell); // empty and multiple-cell snapshots
			std::vector<uint8_t> frame;
			float bounds[6] = {};
			webviewer3d::begin(frame, webviewer3d::Type::Cells, 1, 2);
			assert(frame[4] == 4 && frame[8] == 3);
			webviewer3d::cells(frame, 0, 1, bounds, grid);
			const size_t expected = 32 + 40 + 40 + 20 * grid.m_vCell.size();
			assert(frame.size() == expected);
			assert(frame[36] == grid.m_vCell.size());
			if (!grid.m_vCell.empty())
			{
				assert(std::memcmp(frame.data() + 112, cell.m_ID, 16) == 0);
				assert(frame[128] == 17 && frame[129] == 128 && frame[130] == 255);
				assert(frame[131] == cell.m_vC[3]);
			}
			bool rejected = false;
			try { webviewer3d::points(frame, 1, 2, 1, bounds, {1,2,3}, {1,2,3,255}); }
			catch (const std::invalid_argument &) { rejected = true; }
			assert(rejected && frame.size() == expected);
			OCTGRID_CELLS empty; webviewer3d::cells(frame, 1, 1, bounds, empty);
			assert(frame.size() == expected + 80 && frame[expected] == 1);
		}
	}
	std::cout << "PASS: 20-byte records, depths 0-40 in all octants, canonical IDs, colors and typed streams, wrong-type rejection and object alignment\n";
}
