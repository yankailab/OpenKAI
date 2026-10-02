#include "../../src/DataObject/MavlinkStream.h"
#include <cassert>

namespace
{
	using namespace kai;
	using Messages = vector<std::shared_ptr<MavMsgBase>>;

	void countHeartbeats(void *pMessage, void *pContext)
	{
		auto *pHeartbeat = static_cast<MavHeartbeat *>(pMessage);
		assert(pHeartbeat->get().custom_mode == 42);
		++*static_cast<int *>(pContext);
	}

	mavlink_command_long_t command(const std::shared_ptr<MavMsgBase> &message)
	{
		assert(message->getID() == MAVLINK_MSG_ID_COMMAND_LONG);
		mavlink_message_t encoded = message->encode(255, 190, 1, 2);
		mavlink_command_long_t result{};
		mavlink_msg_command_long_decode(&encoded, &result);
		assert(result.target_system == 1 && result.target_component == 2);
		return result;
	}

	void testReceiveCallbacks(void)
	{
		MavlinkStream input;
		MavlinkStream output;
		auto *pHeartbeat = input.getMsg<MavHeartbeat>();
		assert(pHeartbeat && !pHeartbeat->bValid());
		int received = 0;
		assert(pHeartbeat->addCbRecv(countHeartbeats, &received));
		assert(pHeartbeat->addCbRecv(countHeartbeats, &received));

		mavlink_message_t encoded{};
		mavlink_msg_heartbeat_pack(1, 2, &encoded, MAV_TYPE_QUADROTOR,
			MAV_AUTOPILOT_ARDUPILOTMEGA, MAV_MODE_FLAG_SAFETY_ARMED, 42, MAV_STATE_ACTIVE);
		assert(input.decode(encoded));
		assert(pHeartbeat->bValid() && received == 1);
		assert(!output.getMsg<MavHeartbeat>()->bValid());
		Messages queued;
		input.getMsgQueue(queued);
		assert(queued.empty());

		pHeartbeat->clearCbRecv(countHeartbeats, &received);
		assert(input.decode(encoded));
		assert(received == 1);
	}

	void testOrderedSnapshots(void)
	{
		MavlinkStream output;
		output.clDoSetServo(1, 1200);
		output.clDoSetServo(2, 1800);
		output.clDoSetRelay(3, true);
		Messages firstReader;
		output.getMsgQueue(firstReader);
		assert(firstReader.size() == 3);
		assert(command(firstReader[0]).command == MAV_CMD_DO_SET_SERVO);
		assert(command(firstReader[0]).param1 == 1 && command(firstReader[0]).param2 == 1200);
		assert(command(firstReader[1]).param1 == 2 && command(firstReader[1]).param2 == 1800);
		assert(command(firstReader[2]).command == MAV_CMD_DO_SET_RELAY);
		assert(command(firstReader[2]).param1 == 3 && command(firstReader[2]).param2 == 1);
		for (size_t i = 1; i < firstReader.size(); ++i)
			assert(firstReader[i]->getTstamp() > firstReader[i - 1]->getTstamp());

		const uint64_t cursor = firstReader.back()->getTstamp();
		Messages secondReader;
		output.getMsgQueue(secondReader);
		assert(secondReader.size() == 3);
		output.getMsgQueue(secondReader, cursor);
		assert(secondReader.empty());

		output.clearMsgQueue();
		output.clDoSetServo(4, 1500);
		output.getMsgQueue(secondReader, cursor);
		assert(secondReader.size() == 1);
		assert(command(secondReader[0]).param1 == 4);
		// Clearing/reusing stream storage must not invalidate retained batches.
		assert(command(firstReader[0]).param1 == 1);
		assert(command(firstReader[1]).param2 == 1800);

		output.clearMsgQueue(2);
		for (int servo = 5; servo <= 7; ++servo)
			output.clDoSetServo(servo, 1500);
		output.getMsgQueue(secondReader, cursor);
		assert(secondReader.size() == 2);
		assert(command(secondReader[0]).param1 == 6);
		assert(command(secondReader[1]).param1 == 7);
		assert(command(firstReader[0]).param1 == 1);
	}

	void testIntervalRouting(void)
	{
		MavlinkStream input;
		MavlinkStream output;
		assert(input.setMsgInterval(MAVLINK_MSG_ID_ATTITUDE, NSEC_SEC / 10));
		assert(input.setMsgInterval(MAVLINK_MSG_ID_GLOBAL_POSITION_INT, NSEC_SEC / 5));
		assert(!input.setMsgInterval(-1, NSEC_SEC));
		input.sendSetMsgInterval(&output);

		Messages queued;
		input.getMsgQueue(queued);
		assert(queued.empty());
		output.getMsgQueue(queued);
		assert(queued.size() == 2);
		const auto attitude = command(queued[0]);
		const auto position = command(queued[1]);
		assert(attitude.command == MAV_CMD_SET_MESSAGE_INTERVAL);
		assert(attitude.param1 == MAVLINK_MSG_ID_ATTITUDE && attitude.param2 == 100000);
		assert(position.command == MAV_CMD_SET_MESSAGE_INTERVAL);
		assert(position.param1 == MAVLINK_MSG_ID_GLOBAL_POSITION_INT && position.param2 == 200000);
	}
}

void runMavlinkStreamTests(void)
{
	testReceiveCallbacks();
	testOrderedSnapshots();
	testIntervalRouting();
}
