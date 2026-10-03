#include "../../src/DataObject/MavlinkStream.h"
#include <cassert>

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
		stream.set<MavCommandLong>(servo, 255, 190);
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
		stream.getEncodedMsgs(secondReader, cursor);
		assert(secondReader.empty());

		mavlink_heartbeat_t heartbeat{};
		heartbeat.custom_mode = 7;
		stream.set<MavHeartbeat>(heartbeat, 254, 191);
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
}

void runMavlinkStreamTests(bool testEncoding)
{
	testReceiveCallbacks();
	if (testEncoding)
	{
		testEncodedSnapshots();
		testIntervalRouting();
	}
}
