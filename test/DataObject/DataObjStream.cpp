#include "../../src/DataObject/BBoxStream.h"
#include "../../src/DataObject/BytePacketStream.h"
#include "../../src/DataObject/CANframeStream.h"
#include "../../src/DataObject/UGLIDcellStream.h"
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
		assert(stream.get(elements) == stream.getTstamp());
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
		assert(stream.get(elements) == 1000);
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
		stream.get(elements);
		assert(elements[0].m_tStamp == 10);
		elements[0].m_tStamp = 30;
		stream.get(elements);
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
		assert(stream.get(elements) == 3000);
		assert(elements.size() == 4);
		for (size_t i = 0; i < elements.size(); ++i)
		{
			assert(elements[i].m_tStamp == 22 + i);
		}
		assert(snapshot.size() == 3 && snapshot[0].m_tStamp == 10);
		assert(!stream.clear(static_cast<size_t>(std::numeric_limits<int>::max()) + 1));
		assert(stream.get(elements) == 3000 && elements.size() == 4);

		assert(stream.clear(1));
		stream.get(elements);
		assert(elements.empty());
		// Default publication timestamps use the clock without changing elements.
		const uint64_t before = kai::getTns();
		stream.add(input);
		const uint64_t tStream = stream.get(elements);
		assert(tStream >= before && tStream <= kai::getTns());
		assert(elements.size() == 1 && elements.front().m_tStamp == 25);
		assert(stream.clear(0));
		assert(stream.get(elements) == stream.getTstamp());
		assert(elements.empty());
	}

}

void runDataObjStreamTests(void)
{
	testStream<kai::BBoxStream, kai::BBOX_OBJ>();
	testStream<kai::BytePacketStream, kai::BYTE_PACKET>();
	testStream<kai::CANframeStream, kai::CAN_FRAME>();
	testStream<kai::UGLIDcellStream, kai::UGLID_CELL_T>();
}
