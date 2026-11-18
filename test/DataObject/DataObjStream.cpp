#include "../../src/DataObject/BBoxStream.h"
#include "../../src/DataObject/BytePacketStream.h"
#include "../../src/DataObject/CANframeStream.h"
#include "../../src/DataObject/UGLIDcellStream.h"
#include "../../src/DataObject/PCLframe.h"
#include <cassert>
#include <limits>
#include <type_traits>

namespace
{

	template <class Stream, class Element>
	void testStream(void)
	{
		static_assert(std::is_base_of_v<kai::DataObjStream<Element>, Stream>);
		static_assert(std::is_base_of_v<kai::DataObjBase, Stream>);
		Stream stream;
		std::vector<Element> elements(1);
		assert(stream.get(elements, 0) == stream.getTstamp());
		assert(elements.empty());
		assert(stream.clear(4));

		// Stream and element timestamps are separate, including zero, equal,
		// and out-of-order element timestamps within a single publication.
		std::vector<Element> input(4);
		input[0].m_tStamp = 10;
		input[1].m_tStamp = 10;
		input[2].m_tStamp = 3;
		input[3].m_tStamp = 0;
		stream.add(input, 1000);
		assert(stream.get(elements, 0) == 1000);
		assert(elements.size() == 3);
		assert(elements[0].m_tStamp == 10 && elements[1].m_tStamp == 10);
		assert(elements[2].m_tStamp == 3);
		assert(input[3].m_tStamp == 0);
		assert(stream.get(elements, 3) == 1000);
		assert(elements.size() == 2);
		assert(stream.get(elements, 10) == 1000);
		assert(elements.empty());

		// Neither changing the source nor changing a snapshot changes storage.
		input[0].m_tStamp = 20;
		stream.get(elements, 0);
		assert(elements[0].m_tStamp == 10);
		elements[0].m_tStamp = 30;
		stream.get(elements, 0);
		assert(elements[0].m_tStamp == 10);
		const std::vector<Element> snapshot = elements;
		stream.add({}, 2000);
		assert(stream.getTstamp() == 1000);

		// A batch larger than capacity retains its suffix in arrival order.
		input.resize(6);
		for (size_t i = 0; i < input.size(); ++i)
		{
			input[i].m_tStamp = 20 + i;
		}
		stream.add(input, 3000);
		assert(stream.get(elements, 0) == 3000);
		assert(elements.size() == 4);
		for (size_t i = 0; i < elements.size(); ++i)
		{
			assert(elements[i].m_tStamp == 22 + i);
		}
		assert(snapshot.size() == 3 && snapshot[0].m_tStamp == 10);
		assert(!stream.clear(static_cast<size_t>(std::numeric_limits<int>::max()) + 1));
		assert(stream.get(elements, 0) == 3000 && elements.size() == 4);

		assert(stream.clear(1));
		stream.get(elements, 0);
		assert(elements.empty());
		// Default publication timestamps use the clock without changing elements.
		const uint64_t before = kai::getTns();
		stream.add(input);
		const uint64_t tStream = stream.get(elements, 0);
		assert(tStream >= before && tStream <= kai::getTns());
		assert(elements.size() == 1 && elements.front().m_tStamp == 25);
		assert(stream.clear(0));
		assert(stream.get(elements, 0) == stream.getTstamp());
		assert(elements.empty());
	}

}

void runDataObjStreamTests(void)
{
	testStream<kai::BBoxStream, kai::BBOX_OBJ>();
	testStream<kai::BytePacketStream, kai::BYTE_PACKET>();
	testStream<kai::CANframeStream, kai::CAN_FRAME>();
	testStream<kai::UGLIDcellStream, kai::UGLID_CELL_T>();
	testStream<kai::PCLstream, kai::GEOMETRY_POINT>();

	static_assert(std::is_base_of_v<kai::PCLstream, kai::PCLframe>);
	kai::PCLframe accumulated;
	assert(accumulated.getTstamp() == 0);
	assert(accumulated.clear(3));
	kai::PCLstream &producer = accumulated;
	std::vector<kai::GEOMETRY_POINT> input(2), snapshot;
	input[0].m_tStamp = 7;
	input[0].m_vP.x() = 1;
	input[1].m_tStamp = 0;
	producer.add(input, 100);
	assert(accumulated.get(snapshot) == 100 && snapshot.size() == 3);
	assert(snapshot[0].m_tStamp == 7 && snapshot[1].m_tStamp == 0 && snapshot[2].m_tStamp == 0);
	assert(accumulated.get(snapshot, 0) == 100 && snapshot.size() == 1);
	input[0].m_tStamp = 4;
	input[1].m_tStamp = 9;
	producer.add(input, 101);
	accumulated.get(snapshot);
	assert(snapshot.size() == 3);
	assert(snapshot[0].m_tStamp == 9 && snapshot[1].m_tStamp == 0 && snapshot[2].m_tStamp == 4);
	// Cursor reads still return valid points in arrival order after wrapping.
	accumulated.get(snapshot, 0);
	assert(snapshot.size() == 2 && snapshot[0].m_tStamp == 4 && snapshot[1].m_tStamp == 9);
	assert(accumulated.clear());
	accumulated.get(snapshot);
	assert(snapshot.size() == 3);
	for (const auto &point : snapshot) assert(point.m_tStamp == 0);

	// A separate frame buffer uses whole replacement only. Its storage size
	// matches each input exactly, including invalid timestamps and empty frames.
	kai::PCLframe frame;
	kai::JsonCfg frameConfig;
	frameConfig.setJson({{"class", "PCLframe"}, {"name", "frame"}, {"nBuf", 3}});
	frame.setConfig(&frameConfig, frameConfig.getJson());
	assert(frame.loadConfig());
	input.resize(5);
	for (size_t i = 0; i < input.size(); ++i) input[i].m_tStamp = i;
	frame.set(input, 102);
	assert(frame.get(snapshot) == 102 && snapshot.size() == 5);
	for (size_t i = 0; i < input.size(); ++i) assert(snapshot[i].m_tStamp == i);
	input.resize(1);
	input[0].m_tStamp = 8;
	frame.set(input, 103);
	assert(frame.get(snapshot) == 103 && snapshot.size() == 1 && snapshot[0].m_tStamp == 8);
	input[0].m_tStamp = 90;
	snapshot[0].m_tStamp = 91;
	frame.get(snapshot);
	assert(snapshot[0].m_tStamp == 8);
	frame.set({}, 104);
	assert(frame.get(snapshot) == 104 && snapshot.empty());
	assert(frame.get(snapshot, 0) == 104 && snapshot.empty());

	kai::JsonCfg config;
	config.setJson({{"class", "PCLframe"}, {"name", "cloud"}, {"nBuf", 2}});
	kai::PCLframe initial;
	initial.setConfig(&config, config.getJson());
	assert(initial.loadConfig() && initial.getTstamp() == 0);
	accumulated.setConfig(&config, config.getJson());
	assert(accumulated.loadConfig());
	producer.add({input[0], input[0], input[0]}, 107);
	accumulated.get(snapshot);
	assert(snapshot.size() == 2);
	assert(accumulated.saveConfig());
	assert(config.getJson()->at("nBuf") == 2);
	(*config.getJson())["nBuf"] = static_cast<size_t>(std::numeric_limits<int>::max()) + 1;
	assert(!accumulated.loadConfig());
	accumulated.get(snapshot);
	assert(snapshot.size() == 2); // invalid configuration does not clear storage

	assert(frame.saveConfig());
	assert(frameConfig.getJson()->at("nBuf") == 0);
	initial.setConfig(&frameConfig, frameConfig.getJson());
	assert(initial.loadConfig());
	assert(initial.get(snapshot) == 0 && snapshot.empty());
	assert(initial.get(snapshot, 0) == 0 && snapshot.empty());
	frame.set(input, 108);
	assert(frame.saveConfig());
	assert(frameConfig.getJson()->at("nBuf") == input.size());
}
