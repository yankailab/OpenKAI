// Run with ../run_byte_packet_tests.py after building the WITH_PROTOCOL target.
// WITH_TEST also globs this directory; keep its normal application entry intact.
#ifdef OPENKAI_BYTE_PACKET_PROTOCOL_TEST

#include "../../src/Protocol/_JSONbase.h"
#include "../../src/Protocol/_Mavlink.h"
#include <atomic>
#include <cassert>
#include <iostream>

void runCANStreamTests();
void runBytePacketStreamTests();
void runDataObjStreamTests();
void runMavlinkStreamTests(bool testEncoding);
void runMavlinkConsumerTests(bool testEncoding);

namespace kai
{
	void runBytePacketTransportTests();
	template <class T>
	class _PacketTest : public T
	{
	public:
		_PacketTest(BytePacketStream *pIn, BytePacketStream *pOut = nullptr)
		{
			this->m_pBpStreamIn = pIn;
			this->m_pBpStreamOut = pOut;
		}

		bool check(void) override
		{
			// Exercise parsers without starting module worker threads.
			return this->m_pBpStreamIn || this->m_pBpStreamOut;
		}

		bool readCommand(PROTOCOL_CMD *pCmd)
		{
			return this->readCMD(pCmd);
		}

		bool readJson(string *pText)
		{
			return this->recvJson(pText);
		}

		bool readMavlink(mavlink_message_t *pMsg)
		{
			return this->readMessage(pMsg);
		}
	};

	void addText(BytePacketStream &stream, const string &text)
	{
		stream.add({{vector<uint8_t>(text.begin(), text.end()), getTns()}});
	}

	void testJsonPackets(void)
	{
		BytePacketStream input;
		BytePacketStream output;
		_PacketTest<_JSONbase> reader(&input, &output);
		_PacketTest<_JSONbase> observer(&input);
		string text;
		addText(input, "{\"n\":1}E");
		assert(!reader.readJson(&text));
		assert(text == "{\"n\":1}E");

		addText(input, "OJ{\"n\":2}EOJ{\"n\":3}E");
		assert(reader.readJson(&text));
		assert(text == "{\"n\":1}");
		text.clear();
		assert(reader.readJson(&text));
		assert(text == "{\"n\":2}");
		text.clear();
		assert(!reader.readJson(&text));
		assert(text == "{\"n\":3}E");

		addText(input, "OJ");
		assert(reader.readJson(&text));
		assert(text == "{\"n\":3}");
		text.clear();
		assert(!reader.readJson(&text));
		assert(!reader.readJson(&text));
		assert(text.empty());

		// Another consumer independently sees all messages already read above.
		assert(observer.readJson(&text));
		assert(text == "{\"n\":1}");
		text.clear();
		assert(observer.readJson(&text));
		assert(text == "{\"n\":2}");
		text.clear();
		assert(observer.readJson(&text));
		assert(text == "{\"n\":3}");

		json message = {{"cmd", "test"}};
		assert(reader.sendJson(message));
		vector<BYTE_PACKET> packets;
		output.get(packets);
		assert(packets.size() == 1);
		assert(string(packets[0].m_vB.begin(), packets[0].m_vB.end()) == message.dump());
	}

	void testBinaryPackets(void)
	{
		BytePacketStream input;
		_PacketTest<_ProtocolBase> reader(&input);
		PROTOCOL_CMD cmd;
		cmd.clear();
		input.add({{{PB_BEGIN, 7, 2}, getTns()}});
		assert(!reader.readCommand(&cmd));

		// A split header followed by a complete frame and a partial next frame.
		input.add({{{0, 11, 12, PB_BEGIN, 8, 0, 0, PB_BEGIN, 9, 1, 0}, getTns()}});
		assert(reader.readCommand(&cmd));
		assert(cmd.m_cmd == 7 && cmd.m_nPayload == 2);
		assert(cmd.m_pB[PB_N_HDR] == 11 && cmd.m_pB[PB_N_HDR + 1] == 12);
		cmd.clear();
		assert(reader.readCommand(&cmd));
		assert(cmd.m_cmd == 8 && cmd.m_nPayload == 0);
		cmd.clear();
		assert(!reader.readCommand(&cmd));

		input.add({{{42}, getTns()}});
		assert(reader.readCommand(&cmd));
		assert(cmd.m_cmd == 9 && cmd.m_nPayload == 1 && cmd.m_pB[PB_N_HDR] == 42);
		cmd.clear();
		assert(!reader.readCommand(&cmd));
		assert(!reader.readCommand(&cmd));
	}

	void testMavlinkPackets(void)
	{
		BytePacketStream input;
		_PacketTest<_Mavlink> receiver(&input);
		mavlink_message_t sent;
		mavlink_msg_heartbeat_pack(1, 2, &sent, MAV_TYPE_QUADROTOR,
			MAV_AUTOPILOT_ARDUPILOTMEGA, 0, 1234, MAV_STATE_ACTIVE);
		uint8_t encoded[MAVLINK_MAX_PACKET_LEN];
		const size_t length = mavlink_msg_to_send_buffer(encoded, &sent);
		const vector<uint8_t> frame(encoded, encoded + length);
		input.add({{vector<uint8_t>(frame.begin(), frame.begin() + 4), getTns()}});
		mavlink_message_t received;
		assert(!receiver.readMavlink(&received));
		vector<uint8_t> tail(frame.begin() + 4, frame.end());
		tail.insert(tail.end(), frame.begin(), frame.end());
		input.add({{tail, getTns()}});
		assert(receiver.readMavlink(&received));
		assert(received.msgid == MAVLINK_MSG_ID_HEARTBEAT && received.sysid == 1 && received.compid == 2);
		assert(receiver.readMavlink(&received));
		assert(received.msgid == MAVLINK_MSG_ID_HEARTBEAT);
		assert(!receiver.readMavlink(&received));
		assert(!receiver.readMavlink(&received));
	}

	class MavlinkWorkerHarness : public _Mavlink
	{
	public:
		~MavlinkWorkerHarness()
		{
			joinWorkers();
		}

		void joinWorkers(void)
		{
			if (m_pT)
				m_pT->stop();
			if (m_pTr)
				m_pTr->stop();
			if (m_pT)
				m_pT->join();
			if (m_pTr)
				m_pTr->join();
		}
	};

	void countDecodedHeartbeat(void *, void *pContext)
	{
		++*static_cast<std::atomic<int> *>(pContext);
	}

	void testMavlinkWorkers(bool testEncoding)
	{
		JsonCfg cfg;
		cfg.setJson({
			{"bytesIn", {{"class", "BytePacketStream"}}},
			{"bytesOut", {{"class", "BytePacketStream"}}},
			{"mavlink", {{"class", "MavlinkStream"}}},
			{"codec", {{"class", "_Mavlink"},
				{"BytePacketStreamIn", "bytesIn"}, {"BytePacketStreamOut", "bytesOut"},
				{"MavlinkStream", "mavlink"},
				{"thread", {{"FPS", 500}}}, {"threadR", {{"FPS", 500}}},
				{"devSystemID", 1}, {"devComponentID", 2}, {"iMavComm", MAVLINK_COMM_1}}},
		});
		InstanceMgr instances;
		json &config = *cfg.getJson();
		for (const string name : {"bytesIn", "bytesOut", "mavlink"})
			assert(instances.addDataObject(name, &cfg, &config[name]));
		assert(instances.initAll());
		auto *bytesIn = static_cast<BytePacketStream *>(instances.findDataObject("bytesIn"));
		auto *bytesOut = static_cast<BytePacketStream *>(instances.findDataObject("bytesOut"));
		auto *stream = static_cast<MavlinkStream *>(instances.findDataObject("mavlink"));
		std::atomic<int> heartbeats{0};
		stream->get<MavHeartbeat>()->addCbRecv(countDecodedHeartbeat, &heartbeats);
		MavlinkWorkerHarness codec;
		codec.setName("codec");
		codec.setConfig(&cfg, &config["codec"]);
		assert(codec.loadConfig());
		assert(codec.link(&instances));

		if (testEncoding)
		{
			mavlink_command_long_t servo{};
			servo.target_system = 1;
			servo.target_component = 2;
			servo.command = MAV_CMD_DO_SET_SERVO;
			servo.param1 = 1;
			servo.param2 = 1200;
			stream->set<MavCommandLong>(servo, 255, 190);
		}
		const size_t expectedPackets = testEncoding ? 1 : 0;
		mavlink_message_t heartbeat{};
		mavlink_msg_heartbeat_pack(1, 2, &heartbeat, MAV_TYPE_QUADROTOR,
			MAV_AUTOPILOT_ARDUPILOTMEGA, 0, 99, MAV_STATE_ACTIVE);
		uint8_t frame[MAVLINK_MAX_PACKET_LEN];
		const size_t length = mavlink_msg_to_send_buffer(frame, &heartbeat);
		bytesIn->add({{vector<uint8_t>(frame, frame + 4), getTns()}});
		bytesIn->add({{vector<uint8_t>(frame + 4, frame + length), getTns()}});
		assert(codec.start());

		vector<BYTE_PACKET> packets;
		const uint64_t deadline = getTns() + 2 * NSEC_SEC;
		do
		{
			bytesOut->get(packets);
			if (packets.size() == expectedPackets && heartbeats == 1)
				break;
			::usleep(1000);
		} while (getTns() < deadline);
		codec.joinWorkers();
		bytesOut->get(packets);
		assert(packets.size() == expectedPackets && heartbeats == 1);
		assert(stream->get<MavHeartbeat>()->get().custom_mode == 99);
		for (size_t i = 0; i < packets.size(); ++i)
		{
			mavlink_message_t parser{}, decoded{};
			mavlink_status_t parserStatus{}, status{};
			bool complete = false;
			for (uint8_t byte : packets[i].m_vB)
				complete |= mavlink_frame_char_buffer(&parser, &parserStatus, byte, &decoded, &status) == 1;
			assert(complete && decoded.msgid == MAVLINK_MSG_ID_COMMAND_LONG);
			mavlink_command_long_t command{};
			mavlink_msg_command_long_decode(&decoded, &command);
			assert(command.command == MAV_CMD_DO_SET_SERVO);
			assert(command.param1 == 1 && command.param2 == 1200);
			assert(decoded.sysid == 255 && decoded.compid == 190);
			assert(command.target_system == 1 && command.target_component == 2);
		}
	}
}

#ifdef USE_WSSERVER
namespace kai
{
	void runBytePacketWebSocketTests(void);
}
#endif

int main(int argc, char **argv)
{
	if (argc > 1 && std::string(argv[1]) == "--streams-only")
	{
		runDataObjStreamTests();
		runBytePacketStreamTests();
		std::cout << "DataObjStream timestamp, storage, and concurrency regressions passed\n";
		return 0;
	}
	if (argc > 1 && std::string(argv[1]) == "--can-only")
	{
		runCANStreamTests();
		std::cout << "CAN stream codecs and worker regressions passed\n";
		return 0;
	}
	const bool receiveOnly = argc > 1 && std::string(argv[1]) == "--mavlink-receive-only";
	const bool byteOnly = argc > 1 && std::string(argv[1]) == "--byte-only";
	if (!byteOnly)
	{
		runMavlinkStreamTests(!receiveOnly);
		runMavlinkConsumerTests(!receiveOnly);
		kai::testMavlinkPackets();
		kai::testMavlinkWorkers(!receiveOnly);
	}
	if (receiveOnly)
	{
		std::cout << "MAVLink receive, configuration, and parser regressions passed; "
			"outbound encoding and fragment retention were not tested\n";
		return 0;
	}
	if (argc > 1 && std::string(argv[1]) == "--mavlink-only")
	{
		std::cout << "MavlinkStream storage, codec, and consumer regressions passed\n";
		return 0;
	}
	runDataObjStreamTests();
	runBytePacketStreamTests();
	kai::runBytePacketTransportTests();
#ifdef USE_WSSERVER
	kai::runBytePacketWebSocketTests();
#endif
	kai::testJsonPackets();
	kai::testBinaryPackets();
	runCANStreamTests();
	std::cout << (byteOnly ? "DataObjStream, byte transport, and non-MAVLink protocol regressions passed\n"
		: "DataObjStream and MavlinkStream storage, transport, protocol, and consumer regressions passed\n");
	return 0;
}

#endif
