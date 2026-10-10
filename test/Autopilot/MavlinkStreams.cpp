#ifdef WITH_ARDUPILOT

#include "../../src/Autopilot/FC/ArduPilot/_APmav_base.h"
#include "../../src/Autopilot/FC/ArduPilot/_APmav_RTCM.h"
#include <cassert>
#include <cmath>
#include <array>

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

	struct RTCMAssembly
	{
		vector<uint8_t> bytes;
		std::array<vector<uint8_t>, 4> fragments;
		unsigned received = 0;
		unsigned fragmentCount = 0;
		unsigned sequence = 0;
		unsigned callbacks = 0;

		static void receive(void *message, void *context)
		{
			auto &self = *static_cast<RTCMAssembly *>(context);
			const auto &part = static_cast<MavGpsRTCMdata *>(message)->get();
			assert(part.len <= sizeof(part.data));
			++self.callbacks;
			if (!(part.flags & 1))
			{
				assert(self.received == 0);
				self.bytes.insert(self.bytes.end(), part.data, part.data + part.len);
				return;
			}
			const unsigned fragment = (part.flags >> 1) & 3;
			if (self.received == 0) self.sequence = part.flags >> 3;
			assert(self.sequence == unsigned(part.flags >> 3));
			assert(!(self.received & (1U << fragment)));
			self.fragments[fragment].assign(part.data, part.data + part.len);
			self.received |= 1U << fragment;
			if (part.len < sizeof(part.data)) self.fragmentCount = fragment + 1;
			else if (self.received == 15) self.fragmentCount = 4;
			if (self.fragmentCount && self.received == (1U << self.fragmentCount) - 1)
			{
				for (unsigned i = 0; i < self.fragmentCount; ++i)
					self.bytes.insert(self.bytes.end(), self.fragments[i].begin(), self.fragments[i].end());
				self.received = self.fragmentCount = 0;
			}
		}
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

		uint64_t cursor = 0;
		unsigned sequence = 0;
		const vector<unsigned> lengths = {1, 179, 180, 181, 359, 360, 400,
			539, 540, 541, 719, 720, 721, 900, 1029};
		// Two passes also exercise the five-bit sequence number wrapping.
		for (unsigned iteration = 0; iteration < lengths.size() * 2; ++iteration)
		{
			const unsigned length = lengths[iteration % lengths.size()];
			RTCM_MSG correction;
			correction.init();
			correction.m_nB = length;
			for (size_t i = 0; i < length; ++i)
				correction.m_pB[i] = static_cast<uint8_t>(i + iteration);
			assert(rtcm.writeMavlink(&correction));
			Messages queued;
			const uint64_t next = stream->getEncodedMsgs(queued, cursor);
			assert(next > cursor);
			cursor = next;
			const bool fragmented = length > 180 && length <= 720;
			const unsigned expectedPackets = (length + 179) / 180 +
				(fragmented && length < 720 && length % 180 == 0);
			assert(queued.size() == expectedPackets);
			size_t offset = 0;
			for (size_t i = 0; i < queued.size(); ++i)
			{
				mavlink_gps_rtcm_data_t fragment{};
				mavlink_msg_gps_rtcm_data_decode(&queued[i], &fragment);
				const unsigned seq = fragmented ? sequence : (sequence + i) & 31;
				assert(fragment.flags == ((seq << 3) | (fragmented ? 1 | (i << 1) : 0)));
				assert(fragment.len == std::min<size_t>(180, length - offset));
				assert(memcmp(fragment.data, correction.m_pB + offset, fragment.len) == 0);
				offset += fragment.len;
			}
			assert(offset == length);
			sequence = (sequence + (fragmented ? 1 : expectedPackets)) & 31;

			MavlinkStream receiver;
			RTCMAssembly assembly;
			receiver.get<MavGpsRTCMdata>()->addCbRecv(RTCMAssembly::receive, &assembly);
			// Fragment IDs allow caller-side assembly even when those frames arrive out of order.
			if (fragmented) std::reverse(queued.begin(), queued.end());
			mavlink_message_t parser{}, decoded{};
			mavlink_status_t parserStatus{}, status{};
			for (const auto &encoded : queued)
			{
				uint8_t bytes[MAVLINK_MAX_PACKET_LEN];
				const size_t nBytes = mavlink_msg_to_send_buffer(bytes, &encoded);
				unsigned complete = 0;
				for (size_t i = 0; i < nBytes; ++i)
					if (mavlink_frame_char_buffer(&parser, &parserStatus, bytes[i], &decoded, &status) == MAVLINK_FRAMING_OK)
					{
						assert(receiver.decode(decoded));
						++complete;
					}
				assert(complete == 1);
			}
			assert(assembly.callbacks == expectedPackets && assembly.received == 0);
			assert(assembly.bytes == vector<uint8_t>(correction.m_pB, correction.m_pB + length));
			assert(receiver.getEncodedMsgs(queued) == 0 && queued.empty());
			assert(stream->getEncodedMsgs(queued, cursor) == cursor && queued.empty());
		}
		RTCM_MSG invalid;
		invalid.init();
		assert(!rtcm.writeMavlink(nullptr));
		assert(!rtcm.writeMavlink(&invalid));
		invalid.m_nB = RTCM_N_BUF + 1;
		assert(!rtcm.writeMavlink(&invalid));
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
