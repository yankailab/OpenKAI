// Run with ../run_byte_packet_tests.py after building the WITH_PROTOCOL target.
// WITH_TEST also globs this directory; keep its normal application entry intact.
#ifdef OPENKAI_BYTE_PACKET_PROTOCOL_TEST

#include "../../src/Protocol/_JSONbase.h"
#include "../../src/Protocol/_Mavlink.h"
#include "../../src/Protocol/_USR_CANET.h"
#include <atomic>
#include <cassert>
#include <iostream>

void runBytePacketStreamTests();
void runMavlinkStreamTests();
void runMavlinkConsumerTests();

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

		bool writeMavlink(const mavlink_message_t &msg)
		{
			return this->writeMessage(msg);
		}
	};

	void addText(BytePacketStream &stream, const string &text)
	{
		stream.addPacket(vector<uint8_t>(text.begin(), text.end()));
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
		output.getPackets(packets);
		assert(packets.size() == 1);
		assert(string(packets[0].m_vB.begin(), packets[0].m_vB.end()) == message.dump());
	}

	void testBinaryPackets(void)
	{
		BytePacketStream input;
		_PacketTest<_ProtocolBase> reader(&input);
		PROTOCOL_CMD cmd;
		cmd.clear();
		input.addPacket({PB_BEGIN, 7, 2});
		assert(!reader.readCommand(&cmd));

		// A split header followed by a complete frame and a partial next frame.
		input.addPacket({0, 11, 12, PB_BEGIN, 8, 0, 0, PB_BEGIN, 9, 1, 0});
		assert(reader.readCommand(&cmd));
		assert(cmd.m_cmd == 7 && cmd.m_nPayload == 2);
		assert(cmd.m_pB[PB_N_HDR] == 11 && cmd.m_pB[PB_N_HDR + 1] == 12);
		cmd.clear();
		assert(reader.readCommand(&cmd));
		assert(cmd.m_cmd == 8 && cmd.m_nPayload == 0);
		cmd.clear();
		assert(!reader.readCommand(&cmd));

		input.addPacket({42});
		assert(reader.readCommand(&cmd));
		assert(cmd.m_cmd == 9 && cmd.m_nPayload == 1 && cmd.m_pB[PB_N_HDR] == 42);
		cmd.clear();
		assert(!reader.readCommand(&cmd));
		assert(!reader.readCommand(&cmd));
	}

	void testCanPackets(void)
	{
		BytePacketStream input;
		BytePacketStream output;
		_PacketTest<_USR_CANET> receiver(&input);
		_PacketTest<_USR_CANET> sender(nullptr, &output);
		CAN_F sent;
		sent.clear();
		sent.m_ID = 0x123;
		sent.m_nData = 2;
		sent.m_pData[0] = 10;
		sent.m_pData[1] = 20;
		assert(sender.sendFrame(sent));
		vector<BYTE_PACKET> packets;
		output.getPackets(packets);
		assert(packets.size() == 1 && packets[0].m_vB.size() == CANET_BUF_N);
		const vector<uint8_t> &frame = packets[0].m_vB;
		input.addPacket(vector<uint8_t>(frame.begin(), frame.begin() + 5));
		CAN_F received;
		assert(!receiver.readFrame(&received));
		vector<uint8_t> tail(frame.begin() + 5, frame.end());
		tail.insert(tail.end(), frame.begin(), frame.end());
		input.addPacket(tail);
		assert(receiver.readFrame(&received));
		assert(received.m_nData == 2 && received.m_pData[0] == 10 && received.m_pData[1] == 20);
		assert(receiver.readFrame(&received));
		assert(received.m_nData == 2 && received.m_pData[0] == 10 && received.m_pData[1] == 20);
		assert(!receiver.readFrame(&received));
		assert(!receiver.readFrame(&received));
	}

	void testMavlinkPackets(void)
	{
		BytePacketStream input;
		BytePacketStream output;
		_PacketTest<_Mavlink> receiver(&input);
		_PacketTest<_Mavlink> sender(nullptr, &output);
		mavlink_message_t sent;
		mavlink_msg_heartbeat_pack(1, 2, &sent, MAV_TYPE_QUADROTOR,
			MAV_AUTOPILOT_ARDUPILOTMEGA, 0, 1234, MAV_STATE_ACTIVE);
		assert(sender.writeMavlink(sent));
		vector<BYTE_PACKET> packets;
		output.getPackets(packets);
		assert(packets.size() == 1);
		const vector<uint8_t> &frame = packets[0].m_vB;
		input.addPacket(vector<uint8_t>(frame.begin(), frame.begin() + 4));
		mavlink_message_t received;
		assert(!receiver.readMavlink(&received));
		vector<uint8_t> tail(frame.begin() + 4, frame.end());
		tail.insert(tail.end(), frame.begin(), frame.end());
		input.addPacket(tail);
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

	void testMavlinkWorkers(void)
	{
		JsonCfg cfg;
		cfg.setJson({
			{"bytesIn", {{"class", "BytePacketStream"}}},
			{"bytesOut", {{"class", "BytePacketStream"}}},
			{"commands", {{"class", "MavlinkStream"}}},
			{"received", {{"class", "MavlinkStream"}}},
			{"codec", {{"class", "_Mavlink"},
				{"BytePacketStreamIn", "bytesIn"}, {"BytePacketStreamOut", "bytesOut"},
				{"MavlinkStreamIn", "commands"}, {"MavlinkStreamOut", "received"},
				{"thread", {{"FPS", 500}}}, {"threadR", {{"FPS", 500}}},
				{"devSystemID", 1}, {"devComponentID", 2}, {"iMavComm", MAVLINK_COMM_1}}},
		});
		InstanceMgr instances;
		json &config = *cfg.getJson();
		for (const string name : {"bytesIn", "bytesOut", "commands", "received"})
			assert(instances.addDataObject(name, &cfg, &config[name]));
		assert(instances.initAll());
		auto *bytesIn = static_cast<BytePacketStream *>(instances.findDataObject("bytesIn"));
		auto *bytesOut = static_cast<BytePacketStream *>(instances.findDataObject("bytesOut"));
		auto *commands = static_cast<MavlinkStream *>(instances.findDataObject("commands"));
		auto *received = static_cast<MavlinkStream *>(instances.findDataObject("received"));
		std::atomic<int> heartbeats{0};
		received->getMsg<MavHeartbeat>()->addCbRecv(countDecodedHeartbeat, &heartbeats);
		MavlinkWorkerHarness codec;
		codec.setName("codec");
		codec.setConfig(&cfg, &config["codec"]);
		assert(codec.loadConfig());
		assert(codec.link(&instances));

		commands->clDoSetServo(1, 1200);
		commands->clDoSetServo(2, 1800);
		mavlink_message_t heartbeat{};
		mavlink_msg_heartbeat_pack(1, 2, &heartbeat, MAV_TYPE_QUADROTOR,
			MAV_AUTOPILOT_ARDUPILOTMEGA, 0, 99, MAV_STATE_ACTIVE);
		uint8_t frame[MAVLINK_MAX_PACKET_LEN];
		const size_t length = mavlink_msg_to_send_buffer(frame, &heartbeat);
		bytesIn->addPacket(vector<uint8_t>(frame, frame + 4));
		bytesIn->addPacket(vector<uint8_t>(frame + 4, frame + length));
		assert(codec.start());

		vector<BYTE_PACKET> packets;
		const uint64_t deadline = getTns() + 2 * NSEC_SEC;
		do
		{
			bytesOut->getPackets(packets);
			if (packets.size() == 2 && heartbeats == 1)
				break;
			::usleep(1000);
		} while (getTns() < deadline);
		codec.joinWorkers();
		bytesOut->getPackets(packets);
		assert(packets.size() == 2 && heartbeats == 1);
		assert(received->getMsg<MavHeartbeat>()->get().custom_mode == 99);
		assert(!commands->getMsg<MavHeartbeat>()->bValid());
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
			assert(command.param1 == i + 1 && command.param2 == (i == 0 ? 1200 : 1800));
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
	runMavlinkStreamTests();
	runMavlinkConsumerTests();
	kai::testMavlinkPackets();
	kai::testMavlinkWorkers();
	if (argc > 1 && std::string(argv[1]) == "--mavlink-only")
	{
		std::cout << "MavlinkStream storage, codec, and consumer regressions passed\n";
		return 0;
	}
	runBytePacketStreamTests();
	kai::runBytePacketTransportTests();
#ifdef USE_WSSERVER
	kai::runBytePacketWebSocketTests();
#endif
	kai::testJsonPackets();
	kai::testBinaryPackets();
	kai::testCanPackets();
	std::cout << "BytePacketStream and MavlinkStream storage, transport, protocol, and consumer regressions passed\n";
	return 0;
}

#endif
