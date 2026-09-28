#include "../../src/UI/Viewer/Web/WebGeometrySnapshot.h"
#include <cassert>
#include <fstream>
#include <limits>

namespace
{
	uint32_t read32(const std::vector<uint8_t> &bytes, size_t offset)
	{
		uint32_t value = 0;
		for (size_t i = 0; i < 4; ++i)
		{
			value |= uint32_t(bytes.at(offset + i)) << (8 * i);
		}
		return value;
	}

	std::vector<uint8_t> frame(const kai::WebGeometrySnapshot &snapshot, kai::webselectableoctgrid::Type type)
	{
		std::vector<uint8_t> bytes;
		kai::webselectableoctgrid::begin(bytes, type, 7, 100);
		snapshot.appendTo(bytes);
		kai::webselectableoctgrid::finish(bytes, 1);
		return bytes;
	}

	void write(const char *directory, const char *name, const std::vector<uint8_t> &bytes)
	{
		if (!directory)
		{
			return;
		}
		std::ofstream file(std::string(directory) + "/" + name, std::ios::binary);
		file.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
		assert(file.good());
	}
}

int main(int argc, char **argv)
{
	using namespace kai;
	using webselectableoctgrid::Type;
	PCLframe points;
	LineFrame lines;
	VIEWER_GEOMETRY_SOURCE source;
	source.m_pPCLframe = &points;
	source.m_pLineFrame = &lines;
	source.m_nP = 2;
	source.m_nL = 2;
	source.m_matCol = Vector4f(0, 1, 0, 1);
	WebGeometrySnapshot pointCache;
	WebGeometrySnapshot lineCache;
	const char *directory = argc > 1 ? argv[1] : nullptr;

	assert(pointCache.refresh(source, Type::Points, 3, 0));
	assert(!pointCache.refresh(source, Type::Points, 3, 0));
	assert(read32(frame(pointCache, Type::Points), 36) == 0);

	GEOMETRY_POINT point;
	point.m_vP = Vector3f(1, 2, 3);
	point.m_tStamp = 50;
	GEOMETRY_POINT invalid = point;
	invalid.m_vP.x() = std::numeric_limits<float>::quiet_NaN();
	GEOMETRY_POINT second = point;
	second.m_vP.x() = 4;
	second.m_tStamp = 70;
	GEOMETRY_POINT capped = point;
	capped.m_tStamp = 90;
	points.set({invalid, point, second, capped}, 100);
	assert(pointCache.refresh(source, Type::Points, 3, 0));
	auto bytes = frame(pointCache, Type::Points);
	assert(read32(bytes, 32) == 3);
	assert(read32(bytes, 36) == 2);
	assert(bytes.at(96) == 0 && bytes.at(97) == 255 && bytes.at(98) == 0);
	write(directory, "points.bin", bytes);
	assert(!pointCache.refresh(source, Type::Points, 3, 50));
	assert(pointCache.refresh(source, Type::Points, 3, 51));
	assert(read32(frame(pointCache, Type::Points), 36) == 2);
	assert(pointCache.refresh(source, Type::Points, 3, 91));
	assert(read32(frame(pointCache, Type::Points), 36) == 0);
	assert(!pointCache.refresh(source, Type::Points, 3, 100));

	// Equal timestamps still represent distinct publications, including clear.
	points.set({point}, 100);
	assert(pointCache.refresh(source, Type::Points, 3, 0));
	points.set({}, 100);
	assert(pointCache.refresh(source, Type::Points, 3, 0));
	bytes = frame(pointCache, Type::Points);
	assert(read32(bytes, 36) == 0);
	write(directory, "empty-points.bin", bytes);
	assert(!pointCache.refresh(source, Type::Points, 3, 0));

	GEOMETRY_LINE line;
	line.m_vPa = Vector3f(1, 2, 3);
	line.m_vPb = Vector3f(4, 5, 6);
	line.m_vC = Vector3f(1, 0, 0);
	line.m_tStamp = 60;
	lines.set({line}, 100);
	assert(lineCache.refresh(source, Type::Lines, 3, 0));
	bytes = frame(lineCache, Type::Lines);
	assert(read32(bytes, 36) == 1);
	write(directory, "lines.bin", bytes);
	assert(!lineCache.refresh(source, Type::Lines, 3, 60));
	assert(lineCache.refresh(source, Type::Lines, 3, 61));
	assert(read32(frame(lineCache, Type::Lines), 36) == 0);
	lines.set({}, 100);
	assert(lineCache.refresh(source, Type::Lines, 3, 0));
	write(directory, "empty-lines.bin", frame(lineCache, Type::Lines));
	assert(!lineCache.refresh(source, Type::Lines, 3, 0));
	return 0;
}
