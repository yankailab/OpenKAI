#include "../../src/UI/Viewer/Web/WebSelectableOctGridProtocol.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main()
{
	using namespace kai;
	// RGB payloads need up to three padding bytes before the next object.
	for (auto type : {webselectableoctgrid::Type::Points, webselectableoctgrid::Type::Lines})
	{
		std::vector<uint8_t> frame;
		float bounds[6] = {};
		webselectableoctgrid::begin(frame, type, 1, 2);
		for (size_t count = 0; count <= 5; ++count)
		{
			const size_t vertices = count * (type == webselectableoctgrid::Type::Points ? 1 : 2);
			const size_t start = frame.size();
			const std::vector<float> positions(vertices * 3, 0.5f);
			const std::vector<uint8_t> colors(vertices * 3, 127);
			webselectableoctgrid::vertices(frame, type, uint32_t(count), 2, bounds, positions, colors);
			assert(start % 4 == 0 && frame.size() % 4 == 0);
			assert(frame.size() == start + 40 + (vertices * 15 + 3) / 4 * 4);
			assert(frame[start] == count && frame[start + 4] == count);
			float opacity = 0;
			std::memcpy(&opacity, frame.data() + start + 12, sizeof(opacity));
			assert(opacity == 1);
			for (size_t i = 0; i < colors.size(); ++i)
				assert(frame[start + 40 + positions.size() * 4 + i] == 127);
			for (size_t i = start + 40 + vertices * 15; i < frame.size(); ++i) assert(frame[i] == 0);
		}
		const size_t size = frame.size();
		bool rejected = false;
		try { webselectableoctgrid::vertices(frame, type, 7, 2, bounds, {1,2,3,4,5,6}, {1,2,3,255,4,5,6,255}); }
		catch (const std::invalid_argument &) { rejected = true; }
		assert(rejected && frame.size() == size);
	}
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
			webselectableoctgrid::begin(frame, webselectableoctgrid::Type::Cells, 1, 2);
			assert(frame[4] == 5 && frame[8] == 3);
			webselectableoctgrid::cells(frame, 0, 1, bounds, grid);
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
			try { webselectableoctgrid::points(frame, 1, 2, bounds, {1,2,3}, {1,2,3}); }
			catch (const std::invalid_argument &) { rejected = true; }
			assert(rejected && frame.size() == expected);
			OCTGRID_CELLS empty; webselectableoctgrid::cells(frame, 1, 1, bounds, empty);
			assert(frame.size() == expected + 80 && frame[expected] == 1);
		}
	}
	std::cout << "PASS: 20-byte records, depths 0-40 in all octants, canonical IDs, colors and typed streams, wrong-type rejection and object alignment\n";
}
