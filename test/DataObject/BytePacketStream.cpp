#include "../../src/DataObject/BytePacketStream.h"
#include <cassert>
#include <thread>

namespace
{
	void publishPackets(kai::BytePacketStream *pStream, uint8_t value)
	{
		for (int i = 0; i < 100; ++i)
		{
			pStream->addPacket({value});
		}
	}
}

void runBytePacketStreamTests()
{
	using namespace kai;

	BytePacketStream stream;
	vector<BYTE_PACKET> packets;
	stream.getPackets(packets);
	assert(packets.empty());

	// Construction alone supplies usable storage; reserve size is not a limit.
	const vector<uint8_t> largePacket(8192, 0xa5);
	stream.addPacket(largePacket, 10);
	stream.getPackets(packets);
	assert(packets.size() == 1 && packets.front().m_vB == largePacket);
	assert(packets.front().m_tStamp == 10);

	// Exact cursor equality must not replay packets to a reader.
	stream.getPackets(packets, 10);
	assert(packets.empty());
	vector<BYTE_PACKET> secondReader;
	stream.getPackets(secondReader);
	assert(secondReader.size() == 1);

	assert(stream.clear(3, 2));
	stream.getPackets(packets);
	assert(packets.empty());
	for (uint8_t value = 1; value <= 4; ++value)
	{
		stream.addPacket({value}, 20 + value);
	}
	stream.getPackets(packets);
	assert(packets.size() == 3);
	for (size_t i = 0; i < packets.size(); ++i)
	{
		assert(packets[i].m_vB == vector<uint8_t>{static_cast<uint8_t>(i + 2)});
		assert(packets[i].m_tStamp == i + 22);
	}

	// Snapshots remain valid after the ring overwrites their slots.
	stream.addPacket({5}, 25);
	assert(packets.front().m_vB == vector<uint8_t>{2});
	stream.getPackets(secondReader, 23);
	assert(secondReader.size() == 2);
	assert(secondReader.front().m_tStamp == 24);
	assert(secondReader.back().m_tStamp == 25);

	// Equal/older producer stamps cannot hide a newly published packet.
	stream.addPacket({6}, 25);
	stream.addPacket({7}, 1);
	stream.getPackets(packets, 25);
	assert(packets.size() == 2);
	assert(packets[0].m_tStamp == 26 && packets[1].m_tStamp == 27);
	assert(stream.getTstamp() == 27);
	assert(stream.clear());
	stream.addPacket({8}, 1);
	stream.getPackets(packets, 27);
	assert(packets.size() == 1 && packets.front().m_tStamp == 28);

	// Concurrent publishers/readers take consistent, independent snapshots.
	assert(stream.clear(512));
	std::thread first(publishPackets, &stream, 1);
	std::thread second(publishPackets, &stream, 2);
	uint64_t cursor = 28;
	size_t observed = 0;
	for (int i = 0; i < 100; ++i)
	{
		stream.getPackets(packets, cursor);
		for (const BYTE_PACKET &packet : packets)
		{
			assert(packet.m_tStamp > cursor);
			assert(packet.m_vB.size() == 1);
			cursor = packet.m_tStamp;
			++observed;
		}
		std::this_thread::yield();
	}
	first.join();
	second.join();
	stream.getPackets(packets, cursor);
	observed += packets.size();
	assert(observed == 200);
	stream.getPackets(secondReader, 28);
	assert(secondReader.size() == 200);
	for (size_t i = 1; i < secondReader.size(); ++i)
	{
		assert(secondReader[i].m_tStamp > secondReader[i - 1].m_tStamp);
	}
}
