#include "../../src/DataObject/MavlinkStream.h"
#include <cassert>
#include <atomic>
#include <thread>

namespace
{
	using namespace kai;
	using Messages = vector<mavlink_message_t>;

	void countHeartbeats(void *pMessage, void *pContext)
	{
		auto *pHeartbeat = static_cast<MavHeartbeat *>(pMessage);
		assert(pHeartbeat->get().custom_mode == 42);
		++*static_cast<int *>(pContext);
	}

	mavlink_command_long_t command(const mavlink_message_t &message)
	{
		assert(message.msgid == MAVLINK_MSG_ID_COMMAND_LONG);
		mavlink_command_long_t result{};
		mavlink_msg_command_long_decode(&message, &result);
		return result;
	}

	void testReceiveCallbacks(void)
	{
		MavlinkStream stream;
		MavlinkStream other;
		auto *pHeartbeat = stream.get<MavHeartbeat>();
		assert(pHeartbeat && !pHeartbeat->bValid());
		int received = 0;
		assert(pHeartbeat->addCbRecv(countHeartbeats, &received));
		assert(pHeartbeat->addCbRecv(countHeartbeats, &received));

		mavlink_message_t encoded{};
		mavlink_msg_heartbeat_pack(1, 2, &encoded, MAV_TYPE_QUADROTOR,
			MAV_AUTOPILOT_ARDUPILOTMEGA, MAV_MODE_FLAG_SAFETY_ARMED, 42, MAV_STATE_ACTIVE);
		assert(stream.decode(encoded));
		assert(pHeartbeat->bValid() && received == 1);
		assert(!other.get<MavHeartbeat>()->bValid());
		Messages queued;
		assert(stream.getEncodedMsgs(queued) == 0);
		assert(queued.empty());

		pHeartbeat->clearCbRecv(countHeartbeats, &received);
		assert(stream.decode(encoded));
		assert(received == 1);

		encoded.msgid = 0xFFFFFF;
		assert(!stream.decode(encoded));
		assert(received == 1);
		assert(stream.setMsgInterval(MAVLINK_MSG_ID_ATTITUDE, NSEC_SEC / 10));
		assert(stream.get<MavAttitude>()->getDesiredInterval() == NSEC_SEC / 10);
		assert(!stream.setMsgInterval(-1, NSEC_SEC));
	}

	void testEncodedSnapshots(void)
	{
		MavlinkStream stream;
		mavlink_command_long_t servo{};
		servo.target_system = 1;
		servo.target_component = 2;
		servo.command = MAV_CMD_DO_SET_SERVO;
		servo.param1 = 1;
		servo.param2 = 1200;
		stream.add<MavCommandLong>(servo, 255, 190);
		Messages firstReader;
		const uint64_t cursor = stream.getEncodedMsgs(firstReader);
		assert(cursor > 0 && firstReader.size() == 1);
		assert(firstReader[0].sysid == 255 && firstReader[0].compid == 190);
		const auto firstCommand = command(firstReader[0]);
		assert(firstCommand.command == MAV_CMD_DO_SET_SERVO);
		assert(firstCommand.target_system == 1 && firstCommand.target_component == 2);
		assert(firstCommand.param1 == 1 && firstCommand.param2 == 1200);

		Messages secondReader;
		assert(stream.getEncodedMsgs(secondReader) == cursor);
		assert(secondReader.size() == 1);
		for (int poll = 0; poll < 3; ++poll)
		{
			assert(stream.getEncodedMsgs(secondReader, cursor) == cursor);
			assert(secondReader.empty());
		}

		mavlink_heartbeat_t heartbeat{};
		heartbeat.custom_mode = 7;
		stream.add<MavHeartbeat>(heartbeat, 254, 191);
		assert(stream.getEncodedMsgs(secondReader, cursor) > cursor);
		assert(secondReader.size() == 1);
		assert(secondReader[0].msgid == MAVLINK_MSG_ID_HEARTBEAT);
		assert(secondReader[0].sysid == 254 && secondReader[0].compid == 191);
		mavlink_heartbeat_t decoded{};
		mavlink_msg_heartbeat_decode(&secondReader[0], &decoded);
		assert(decoded.custom_mode == 7);
		// Outbound messages do not overwrite telemetry received through decode().
		assert(!stream.get<MavHeartbeat>()->bValid());
		assert(command(firstReader[0]).param2 == 1200);
	}

	void testIntervalRouting(void)
	{
		MavlinkStream stream;
		assert(stream.setMsgInterval(MAVLINK_MSG_ID_ATTITUDE, NSEC_SEC / 10));
		stream.sendSetMsgInterval();

		Messages queued;
		stream.getEncodedMsgs(queued);
		assert(queued.size() == 1);
		const auto attitude = command(queued[0]);
		assert(attitude.command == MAV_CMD_SET_MESSAGE_INTERVAL);
		assert(attitude.param1 == MAVLINK_MSG_ID_ATTITUDE && attitude.param2 == 100000);
	}

	class QueueHarness : public MavlinkStream
	{
	public:
		using MavlinkStream::clearMsgQueue;
	};

	void testQueueWrap(void)
	{
		QueueHarness stream;
		stream.clearMsgQueue(3);
		mavlink_heartbeat_t heartbeat{};
		Messages messages;
		uint64_t cursor = 0;
		for (unsigned i = 0; i < 8; ++i)
		{
			heartbeat.custom_mode = i;
			stream.add<MavHeartbeat>(heartbeat);
			const uint64_t next = stream.getEncodedMsgs(messages, cursor);
			assert(next > cursor && messages.size() == 1);
			cursor = next;
		}
		Messages observer;
		assert(stream.getEncodedMsgs(observer) == cursor);
		assert(observer.size() == 3);
		for (size_t i = 0; i < observer.size(); ++i)
		{
			mavlink_heartbeat_t decoded{};
			mavlink_msg_heartbeat_decode(&observer[i], &decoded);
			assert(decoded.custom_mode == i + 5);
		}
		stream.clearMsgQueue(1);
		assert(stream.getEncodedMsgs(messages, cursor) == cursor && messages.empty());
		heartbeat.custom_mode = 8;
		stream.add<MavHeartbeat>(heartbeat);
		assert(stream.getEncodedMsgs(messages, cursor) > cursor && messages.size() == 1);
		assert(observer.size() == 3); // A copied snapshot survives overwrites and resizing.
	}

	void testConcurrentQueue(void)
	{
		MavlinkStream stream;
		constexpr unsigned producers = 4;
		constexpr unsigned perProducer = 200;
		std::atomic<bool> start{false};
		std::atomic<unsigned> done{0};
		vector<std::thread> workers;
		for (unsigned p = 0; p < producers; ++p)
		{
			workers.emplace_back([&, p] {
				while (!start.load()) std::this_thread::yield();
				for (unsigned i = 0; i < perProducer; ++i)
				{
					if (p % 2 == 0)
					{
						mavlink_heartbeat_t heartbeat{};
						heartbeat.custom_mode = p * perProducer + i;
						stream.add<MavHeartbeat>(heartbeat, p + 1, 190);
					}
					else
					{
						mavlink_command_long_t command{};
						command.param1 = p * perProducer + i;
						stream.add<MavCommandLong>(command, p + 1, 190);
					}
				}
				++done;
			});
		}
		for (unsigned reader = 0; reader < 2; ++reader)
		{
			workers.emplace_back([&] {
				while (!start.load()) std::this_thread::yield();
				vector<unsigned> counts(producers, 0);
				uint64_t cursor = 0;
				Messages messages;
				bool finished = false;
				while (!finished)
				{
					finished = done.load() == producers;
					const uint64_t next = stream.getEncodedMsgs(messages, cursor);
					assert(next >= cursor);
					assert(messages.empty() || next > cursor);
					cursor = next;
					for (const auto &encoded : messages)
					{
						const unsigned p = encoded.sysid - 1;
						assert(p < producers && encoded.compid == 190);
						unsigned value;
						if (encoded.msgid == MAVLINK_MSG_ID_HEARTBEAT)
						{
							mavlink_heartbeat_t heartbeat{};
							mavlink_msg_heartbeat_decode(&encoded, &heartbeat);
							value = heartbeat.custom_mode;
						}
						else value = command(encoded).param1;
						assert(value == p * perProducer + counts[p]);
						++counts[p];
					}
					std::this_thread::yield();
				}
				for (unsigned count : counts) assert(count == perProducer);
			});
		}
		start = true;
		for (auto &worker : workers) worker.join();
	}
}

void runMavlinkStreamTests(bool testEncoding)
{
	testReceiveCallbacks();
	if (testEncoding)
	{
		testEncodedSnapshots();
		testIntervalRouting();
		testQueueWrap();
		testConcurrentQueue();
	}
}
