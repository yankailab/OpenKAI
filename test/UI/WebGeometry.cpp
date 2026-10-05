#include "../../src/UI/Viewer/Web/_WebGeometry.h"
#include "../../src/UI/Viewer/Web/_WebSelectableOctGrid.h"
#include "../../src/Instance/InstanceMgr.h"
#include <cassert>
#include <cstring>
#include <iostream>

namespace
{
	using namespace kai;
	namespace wire = webselectableoctgrid;

	uint32_t readU32(const std::vector<uint8_t> &bytes, size_t at)
	{
		uint32_t value = 0;
		for (unsigned i = 0; i < 4; ++i) value |= uint32_t(bytes.at(at + i)) << (i * 8);
		return value;
	}
	float readFloat(const std::vector<uint8_t> &bytes, size_t at)
	{
		const uint32_t value = readU32(bytes, at);
		float result;
		std::memcpy(&result, &value, sizeof(result));
		return result;
	}

	template<class Viewer>
	void linkTest(JsonCfg &cfg, json &config, InstanceMgr &instances)
	{
		Viewer viewer;
		viewer.setName("viewer");
		viewer.setConfig(&cfg, &config);
		assert(viewer.loadConfig() && viewer.link(&instances));
		// A large cap still fails at link time when its worst-case frame cannot fit.
		config["nPbuf"] = 20000000;
		config["vGeometry"][1]["nP"] = 20000000;
		assert(viewer.loadConfig() && !viewer.link(&instances));
	}

	void configuredCapsTest()
	{
		JsonCfg cfg;
		assert(cfg.readFromFile("jsonCfg/WebViewer3D.json"));
		json &config = *cfg.getJson();
		InstanceMgr instances;
		for (const char *name : {"pcFilePoints", "scPC"})
			assert(instances.addDataObject(name, &cfg, &config[name]));
		assert(instances.addModule("octGrid", &cfg, &config["octGrid"]));
		assert(instances.initAll());

		const json original = config["viewer"];
		SelectableOctGridSources sources;
		std::string error;
		const int nP = original.at("nPbuf"), nL = original.at("nLbuf");
		assert(sources.link(original, &instances, nP, nL, 18333, error));
		assert(sources.m_vGeometry.size() == 2);
		assert(sources.m_vGeometry[0].m_name == "scPC" && sources.m_vGeometry[0].m_nP == 400000);
		assert(sources.m_vGeometry[1].m_name == "pcFilePoints" && sources.m_vGeometry[1].m_nP == 9000000);

		assert(sources.link(original, &instances, 123, nL, 18333, error));
		for (const auto &source : sources.m_vGeometry) assert(source.m_nP == 123);
		assert(sources.link(original, &instances, 0, nL, 18333, error));
		for (const auto &source : sources.m_vGeometry) assert(source.m_nP == 0);
		json disabled = original;
		disabled["vGeometry"][1]["nP"] = 0;
		assert(sources.link(disabled, &instances, nP, nL, 18333, error));
		assert(sources.m_vGeometry[0].m_nP == 400000 && sources.m_vGeometry[1].m_nP == 0);

		linkTest<_WebSelectableOctGrid>(cfg, config["viewer"], instances);
		config["viewer"] = original;
		config["viewer"].erase("vSelectableOctGrid");
		config["viewer"].erase("nCbuf");
		config["viewer"]["class"] = "_WebGeometry";
		linkTest<_WebGeometry>(cfg, config["viewer"], instances);
	}

	void largeFrameTest()
	{
		// Just over the former 64 MiB wire limit, with markers beyond the old 400k cap.
		constexpr size_t count = 4500001;
		std::vector<float> positions(count * 3, 0);
		std::vector<uint8_t> colors(count * 3, 255);
		positions[400000 * 3] = 17;
		positions[(count - 1) * 3] = 101;
		positions[(count - 1) * 3 + 1] = -202;
		positions[(count - 1) * 3 + 2] = 303;
		colors[(count - 1) * 3] = 11;
		colors[(count - 1) * 3 + 1] = 22;
		colors[(count - 1) * 3 + 2] = 33;
		const float bounds[6] = {0, -202, 0, 101, 0, 303};
		std::vector<uint8_t> frame;
		frame.reserve(wire::HeaderBytes + wire::ObjectBytes + wire::vertexBytes(count));
		wire::begin(frame, wire::Type::Points, 7, 123);
		wire::points(frame, 1, 2, bounds, positions, colors);
		wire::finish(frame, 1);
		assert(frame.size() > 64 * 1024 * 1024 && frame.size() <= wire::MaxFrameBytes);
		assert(readU32(frame, 16) == 1 && readU32(frame, 20) == frame.size());
		assert(readU32(frame, wire::HeaderBytes + 4) == count);
		const size_t payload = wire::HeaderBytes + wire::ObjectBytes;
		assert(readFloat(frame, payload + 400000 * 12) == 17);
		for (size_t axis = 0; axis < 3; ++axis)
		{
			const size_t component = (count - 1) * 3 + axis;
			assert(readFloat(frame, payload + component * 4) == positions[component]);
			assert(frame.at(payload + positions.size() * 4 + component) == colors[component]);
		}
		assert(frame.back() == 0); // RGB payload padding is retained after the final point.
	}

	void oversizedFrameTest()
	{
		std::vector<uint8_t> frame;
		wire::begin(frame, wire::Type::Points, 0, 0);
		const auto before = frame;
		const size_t capacity = frame.capacity();
		const float bounds[6] = {};
		const size_t largestPayload = wire::MaxFrameBytes - wire::HeaderBytes - wire::ObjectBytes;
		for (size_t payload : {largestPayload + 1, wire::MaxFrameBytes + 1})
		{
			bool rejected = false;
			try { wire::objectHeader(frame, wire::Type::Points, 0, 1, 2, 1, bounds, payload); }
			catch (const std::length_error &) { rejected = true; }
			assert(rejected && frame == before && frame.capacity() == capacity);
		}
	}
}

int main()
{
	configuredCapsTest();
	largeFrameTest();
	oversizedFrameTest();
	std::cout << "Web geometry configured caps, large point frames and frame limits passed\n";
}
