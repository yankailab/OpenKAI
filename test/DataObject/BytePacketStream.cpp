#include "../../src/DataObject/BytePacketStream.h"
#include <cassert>
#include <thread>

namespace
{
	void publishPackets(kai::BytePacketStream *pStream, uint8_t value)
	{
		for (int i = 0; i < 100; ++i)
		{
			pStream->add({{{value}, static_cast<uint64_t>(100 + value)}}, 1000 + value);
		}
	}
}

void runBytePacketStreamTests()
{
	using namespace kai;

	BytePacketStream stream;
	vector<BYTE_PACKET> packets;
	stream.get(packets);
	assert(packets.empty());

	// Construction alone supplies usable storage; reserve size is not a limit.
	const vector<uint8_t> largePacket(8192, 0xa5);
	stream.add({{largePacket, 10}}, 1000);
	stream.get(packets);
	assert(packets.size() == 1 && packets.front().m_vB == largePacket);
	assert(packets.front().m_tStamp == 10);

	// Exact cursor equality must not replay packets to a reader.
	stream.get(packets, 10);
	assert(packets.empty());
	vector<BYTE_PACKET> secondReader;
	stream.get(secondReader);
	assert(secondReader.size() == 1);

	assert(stream.clear(3, 2));
	stream.get(packets);
	assert(packets.empty());
	for (uint8_t value = 1; value <= 4; ++value)
	{
		stream.add({{{value}, static_cast<uint64_t>(20 + value)}});
	}
	stream.get(packets);
	assert(packets.size() == 3);
	for (size_t i = 0; i < packets.size(); ++i)
	{
		assert(packets[i].m_vB == vector<uint8_t>{static_cast<uint8_t>(i + 2)});
		assert(packets[i].m_tStamp == i + 22);
	}

	// Snapshots remain valid after the ring overwrites their slots.
	stream.add({{{5}, 25}});
	assert(packets.front().m_vB == vector<uint8_t>{2});
	stream.get(secondReader, 23);
	assert(secondReader.size() == 2);
	assert(secondReader.front().m_tStamp == 24);
	assert(secondReader.back().m_tStamp == 25);

	// Equal/older producer stamps are retained unchanged, even across clear.
	stream.add({{{6}, 25}, {{7}, 1}}, 2000);
	assert(stream.get(packets, 25) == 2000);
	assert(packets.empty());
	stream.get(packets);
	assert(packets.size() == 3);
	assert(packets[1].m_tStamp == 25 && packets[2].m_tStamp == 1);
	assert(stream.clear());
	stream.add({{{8}, 1}}, 2001);
	assert(stream.get(packets) == 2001);
	assert(packets.size() == 1 && packets.front().m_tStamp == 1);

	// The input and output payloads are independently owned copies.
	vector<BYTE_PACKET> input{{{9, 10}, 5}};
	stream.add(input);
	input.front().m_vB[0] = 0;
	stream.get(packets, 1);
	assert(packets.size() == 1 && packets.front().m_vB[0] == 9);
	packets.front().m_vB[0] = 0;
	stream.get(packets, 1);
	assert(packets.front().m_vB[0] == 9);

	// Concurrent publishers/readers take consistent, independent snapshots.
	// Timestamp ordering belongs to the producers, so observe complete snapshots.
	assert(stream.clear(512));
	std::thread first(publishPackets, &stream, 1);
	std::thread second(publishPackets, &stream, 2);
	size_t previousSize = 0;
	for (int i = 0; i < 100; ++i)
	{
		const uint64_t tStream = stream.get(packets);
		assert(packets.size() >= previousSize);
		previousSize = packets.size();
		for (const BYTE_PACKET &packet : packets)
		{
			assert(packet.m_vB.size() == 1);
			assert(packet.m_tStamp == static_cast<uint64_t>(100 + packet.m_vB[0]));
		}
		if (!packets.empty())
		{
			assert(tStream == static_cast<uint64_t>(1000 + packets.back().m_vB[0]));
		}
		std::this_thread::yield();
	}
	first.join();
	second.join();
	stream.get(packets);
	assert(packets.size() == 200);
	stream.get(secondReader);
	assert(secondReader.size() == 200);
	size_t counts[3]{};
	for (const BYTE_PACKET &packet : packets)
	{
		assert(packet.m_vB.size() == 1);
		assert(packet.m_vB[0] == 1 || packet.m_vB[0] == 2);
		assert(packet.m_tStamp == static_cast<uint64_t>(100 + packet.m_vB[0]));
		++counts[packet.m_vB[0]];
	}
	assert(counts[1] == 100 && counts[2] == 100);
}
