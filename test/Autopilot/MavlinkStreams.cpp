#ifdef WITH_ARDUPILOT

#include "../../src/Autopilot/FC/ArduPilot/_APmav_base.h"
#include "../../src/Autopilot/FC/ArduPilot/_APmav_RTCM.h"
#include <cassert>
#include <cmath>

namespace
{
	using namespace kai;
	using Messages = vector<mavlink_message_t>;

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

	void testAutopilotStreams(bool testEncoding)
	{
		JsonCfg cfg;
		cfg.setJson({
			{"mavlink", {{"class", "MavlinkStream"}}},
			{"autopilot", {{"class", "_APmav_base"},
				{"MavlinkStream", "mavlink"},
				{"mavMsgInt", {{"attitude", {{"id", MAVLINK_MSG_ID_ATTITUDE}, {"tInt", 0.1}}}}}}},
		});
		InstanceMgr instances;
		json &config = *cfg.getJson();
		assert(instances.addDataObject("mavlink", &cfg, &config["mavlink"]));
		assert(instances.initAll());
		AutopilotHarness autopilot;
		autopilot.setName("autopilot");
		autopilot.setConfig(&cfg, &config["autopilot"]);
		assert(autopilot.loadConfig());
		assert(autopilot.link(&instances));
		assert(autopilot.check());
		auto *stream = static_cast<MavlinkStream *>(instances.findDataObject("mavlink"));
		assert(autopilot.getMavlinkStream() == stream);
		assert(stream->get<MavAttitude>()->getDesiredInterval() == NSEC_SEC / 10);

		mavlink_message_t encoded{};
		mavlink_msg_heartbeat_pack(1, 2, &encoded, MAV_TYPE_QUADROTOR,
			MAV_AUTOPILOT_ARDUPILOTMEGA, 0, 7, MAV_STATE_ACTIVE);
		assert(stream->decode(encoded));
		mavlink_attitude_t attitude{};
		attitude.roll = 0.25;
		attitude.pitch = -0.5;
		attitude.yaw = 1.0;
		mavlink_msg_attitude_encode(1, 2, &encoded, &attitude);
		assert(stream->decode(encoded));
		autopilot.updateApMavRecv();
		assert(autopilot.getArm() == apArm_disarm);
		assert(autopilot.getCustomMode() == 7);
		assert(std::abs(autopilot.getAngles().x() - 0.25) < 1e-6);
		assert(std::abs(autopilot.getAngles().y() + 0.5) < 1e-6);
		assert(std::abs(autopilot.getAngles().z() - 1.0) < 1e-6);

		if (!testEncoding)
			return;

		autopilot.requestArm();
		autopilot.setCustomMode(7);
		autopilot.updateApMavSend();
		Messages queued;
		stream->getEncodedMsgs(queued);
		bool armed = false;
		for (const auto &message : queued)
		{
			if (message.msgid != MAVLINK_MSG_ID_COMMAND_LONG)
				continue;
			encoded = message;
			mavlink_command_long_t command{};
			mavlink_msg_command_long_decode(&encoded, &command);
			if (command.command == MAV_CMD_COMPONENT_ARM_DISARM)
			{
				assert(command.param1 == 1);
				armed = true;
			}
		}
		assert(armed);
		assert(stream->get<MavHeartbeat>()->get().custom_mode == 7);

		// One operation queues two PARAM_SET values; both must survive until encoding.
		const uint64_t cursor = stream->getEncodedMsgs(queued);
		AP_MOUNT mount{};
		mount.init();
		mount.m_config.stab_pitch = 1;
		mount.m_config.stab_roll = 0;
		autopilot.setMount(mount);
		stream->getEncodedMsgs(queued, cursor);
		assert(queued.size() == 4);
		bool tilt = false;
		bool roll = false;
		for (const auto &message : queued)
		{
			if (message.msgid != MAVLINK_MSG_ID_PARAM_SET)
				continue;
			mavlink_param_set_t parameter{};
			mavlink_msg_param_set_decode(&message, &parameter);
			if (string(parameter.param_id) == "MNT_STAB_TILT")
			{
				assert(parameter.param_value == 1);
				tilt = true;
			}
			if (string(parameter.param_id) == "MNT_STAB_ROLL")
			{
				assert(parameter.param_value == 0);
				roll = true;
			}
		}
		assert(tilt && roll);
	}

	void testRTCMStreams(bool testEncoding)
	{
		JsonCfg cfg;
		cfg.setJson({
			{"mavlink", {{"class", "MavlinkStream"}}},
			{"corrections", {{"class", "BytePacketStream"}}},
			{"rtcm", {{"class", "_APmav_RTCM"},
				{"BytePacketStreamIn", "corrections"},
				{"MavlinkStream", "mavlink"}}},
		});
		InstanceMgr instances;
		json &config = *cfg.getJson();
		for (const string name : {"mavlink", "corrections"})
			assert(instances.addDataObject(name, &cfg, &config[name]));
		assert(instances.initAll());
		RTCMHarness rtcm;
		rtcm.setName("rtcm");
		rtcm.setConfig(&cfg, &config["rtcm"]);
		assert(rtcm.loadConfig());
		assert(rtcm.link(&instances));
		assert(rtcm.check());
		auto *stream = static_cast<MavlinkStream *>(instances.findDataObject("mavlink"));
		if (!testEncoding)
			return;

		RTCM_MSG correction;
		correction.init();
		correction.m_nB = 400;
		for (size_t i = 0; i < correction.m_nB; ++i)
			correction.m_pB[i] = static_cast<uint8_t>(i);
		assert(rtcm.writeMavlink(&correction));
		Messages queued;
		stream->getEncodedMsgs(queued);
		assert(queued.size() == 3);
		size_t offset = 0;
		for (size_t i = 0; i < queued.size(); ++i)
		{
			const mavlink_message_t &encoded = queued[i];
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

void runMavlinkConsumerTests(bool testEncoding)
{
#ifdef WITH_ARDUPILOT
	testAutopilotStreams(testEncoding);
	testRTCMStreams(testEncoding);
#endif
}
