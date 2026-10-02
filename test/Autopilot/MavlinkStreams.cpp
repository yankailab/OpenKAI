#ifdef WITH_ARDUPILOT

#include "../../src/Autopilot/FC/ArduPilot/_APmav_base.h"
#include "../../src/Autopilot/FC/ArduPilot/_APmav_RTCM.h"
#include <cassert>
#include <cmath>

namespace
{
	using namespace kai;
	using Messages = vector<std::shared_ptr<MavMsgBase>>;

	class AutopilotHarness : public _APmav_base
	{
	public:
		using _APmav_base::updateApMavRecv;
		using _APmav_base::updateApMavSend;

		void requestArm(void)
		{
			m_arm = apArm_arm;
		}
	};

	class RTCMHarness : public _APmav_RTCM
	{
	public:
		using _APmav_RTCM::writeMavlink;
	};

	void testAutopilotStreams(void)
	{
		JsonCfg cfg;
		cfg.setJson({
			{"received", {{"class", "MavlinkStream"}}},
			{"commands", {{"class", "MavlinkStream"}}},
			{"autopilot", {{"class", "_APmav_base"},
				{"MavlinkStreamIn", "received"}, {"MavlinkStreamOut", "commands"},
				{"mavMsgInt", {{"attitude", {{"id", MAVLINK_MSG_ID_ATTITUDE}, {"tInt", 0.1}}}}}}},
		});
		InstanceMgr instances;
		json &config = *cfg.getJson();
		assert(instances.addDataObject("received", &cfg, &config["received"]));
		assert(instances.addDataObject("commands", &cfg, &config["commands"]));
		assert(instances.initAll());
		AutopilotHarness autopilot;
		autopilot.setName("autopilot");
		autopilot.setConfig(&cfg, &config["autopilot"]);
		assert(autopilot.loadConfig());
		assert(autopilot.link(&instances));
		assert(autopilot.check());
		auto *input = static_cast<MavlinkStream *>(instances.findDataObject("received"));
		auto *output = static_cast<MavlinkStream *>(instances.findDataObject("commands"));
		assert(autopilot.getMavlinkStreamIn() == input);
		assert(autopilot.getMavlinkStreamOut() == output);
		assert(input->getMsg<MavAttitude>()->getDesiredInterval() == NSEC_SEC / 10);

		mavlink_message_t encoded{};
		mavlink_msg_heartbeat_pack(1, 2, &encoded, MAV_TYPE_QUADROTOR,
			MAV_AUTOPILOT_ARDUPILOTMEGA, 0, 7, MAV_STATE_ACTIVE);
		assert(input->decode(encoded));
		mavlink_attitude_t attitude{};
		attitude.roll = 0.25;
		attitude.pitch = -0.5;
		attitude.yaw = 1.0;
		mavlink_msg_attitude_encode(1, 2, &encoded, &attitude);
		assert(input->decode(encoded));
		autopilot.updateApMavRecv();
		assert(autopilot.getArm() == apArm_disarm);
		assert(autopilot.getCustomMode() == 7);
		assert(std::abs(autopilot.getAngles().x() - 0.25) < 1e-6);
		assert(std::abs(autopilot.getAngles().y() + 0.5) < 1e-6);
		assert(std::abs(autopilot.getAngles().z() - 1.0) < 1e-6);

		autopilot.requestArm();
		autopilot.setCustomMode(7);
		autopilot.updateApMavSend();
		Messages queued;
		input->getMsgQueue(queued);
		assert(queued.empty());
		output->getMsgQueue(queued);
		bool armed = false;
		for (const auto &message : queued)
		{
			if (message->getID() != MAVLINK_MSG_ID_COMMAND_LONG)
				continue;
			encoded = message->encode(255, 190, 1, 2);
			mavlink_command_long_t command{};
			mavlink_msg_command_long_decode(&encoded, &command);
			if (command.command == MAV_CMD_COMPONENT_ARM_DISARM)
			{
				assert(command.param1 == 1);
				armed = true;
			}
		}
		assert(armed);
		assert(input->getMsg<MavHeartbeat>()->get().custom_mode == 7);

		// One operation queues two PARAM_SET values; both must survive until encoding.
		output->clearMsgQueue();
		AP_MOUNT mount{};
		mount.init();
		mount.m_config.stab_pitch = 1;
		mount.m_config.stab_roll = 0;
		autopilot.setMount(mount);
		output->getMsgQueue(queued);
		assert(queued.size() == 4);
		mavlink_param_set_t parameter{};
		encoded = queued[2]->encode(255, 190, 1, 2);
		mavlink_msg_param_set_decode(&encoded, &parameter);
		assert(string(parameter.param_id) == "MNT_STAB_TILT" && parameter.param_value == 1);
		encoded = queued[3]->encode(255, 190, 1, 2);
		mavlink_msg_param_set_decode(&encoded, &parameter);
		assert(string(parameter.param_id) == "MNT_STAB_ROLL" && parameter.param_value == 0);
	}

	void testRTCMFragments(void)
	{
		JsonCfg cfg;
		cfg.setJson({
			{"received", {{"class", "MavlinkStream"}}},
			{"commands", {{"class", "MavlinkStream"}}},
			{"corrections", {{"class", "BytePacketStream"}}},
			{"rtcm", {{"class", "_APmav_RTCM"},
				{"BytePacketStreamIn", "corrections"},
				{"MavlinkStreamIn", "received"}, {"MavlinkStreamOut", "commands"}}},
		});
		InstanceMgr instances;
		json &config = *cfg.getJson();
		for (const string name : {"received", "commands", "corrections"})
			assert(instances.addDataObject(name, &cfg, &config[name]));
		assert(instances.initAll());
		RTCMHarness rtcm;
		rtcm.setName("rtcm");
		rtcm.setConfig(&cfg, &config["rtcm"]);
		assert(rtcm.loadConfig());
		assert(rtcm.link(&instances));
		assert(rtcm.check());
		auto *input = static_cast<MavlinkStream *>(instances.findDataObject("received"));
		auto *output = static_cast<MavlinkStream *>(instances.findDataObject("commands"));

		RTCM_MSG correction;
		correction.init();
		correction.m_nB = 400;
		for (size_t i = 0; i < correction.m_nB; ++i)
			correction.m_pB[i] = static_cast<uint8_t>(i);
		assert(rtcm.writeMavlink(&correction));
		Messages queued;
		input->getMsgQueue(queued);
		assert(queued.empty());
		output->getMsgQueue(queued);
		assert(queued.size() == 3);
		size_t offset = 0;
		for (size_t i = 0; i < queued.size(); ++i)
		{
			mavlink_message_t encoded = queued[i]->encode(255, 190, 1, 2);
			mavlink_gps_rtcm_data_t fragment{};
			mavlink_msg_gps_rtcm_data_decode(&encoded, &fragment);
			assert(fragment.flags == (1 | (i << 1)));
			assert(fragment.len == (i < 2 ? 180 : 40));
			assert(memcmp(fragment.data, correction.m_pB + offset, fragment.len) == 0);
			offset += fragment.len;
		}
		assert(offset == correction.m_nB);
	}
}

#endif

void runMavlinkConsumerTests(void)
{
#ifdef WITH_ARDUPILOT
	testAutopilotStreams();
	testRTCMFragments();
#endif
}
