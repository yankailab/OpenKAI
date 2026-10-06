// Run with test/run_byte_packet_tests.py --can-only after a WITH_PROTOCOL build.
#ifdef OPENKAI_BYTE_PACKET_PROTOCOL_TEST

#include "../../src/Protocol/_SocketCAN.h"
#include "../../src/Protocol/_USR_CANET.h"
#include <atomic>
#include <cassert>
#include <cerrno>
#include <fcntl.h>

namespace
{
	using namespace kai;

	class FailingCANthread : public _Thread
	{
	public:
		bool startThread(void *(*pStart)(void *), void *pArg) override
		{
			m_bAttempted = true;
			m_setState = thread_run;
			return false;
		}

		bool m_bAttempted = false;
	};

	template <class Codec>
	class CANworkerHarness : public Codec
	{
	public:
		~CANworkerHarness()
		{
			this->stop();
		}

		using Codec::getThread;

		bool sendFrame(void) override
		{
			m_sendThread = pthread_self();
			++m_nSends;
			return Codec::sendFrame();
		}

		bool readFrame(void) override
		{
			m_readThread = pthread_self();
			++m_nReads;
			return Codec::readFrame();
		}

		void setStreams(CANframeStream *pIn, CANframeStream *pOut)
		{
			this->m_pCANframeIn = pIn;
			this->m_pCANframeOut = pOut;
		}

		FailingCANthread *failWorker(bool bRead)
		{
			_Thread *&pThread = bRead ? this->m_pTr : this->m_pT;
			delete pThread;
			FailingCANthread *pFailure = new FailingCANthread();
			pThread = pFailure;
			pThread->setName(bRead ? "threadR" : "thread");
			return pFailure;
		}

		size_t passes(bool bRead) const
		{
			return bRead ? m_nReads.load() : m_nSends.load();
		}

		void waitPasses(bool bRead, size_t previous)
		{
			const uint64_t deadline = getTns() + 2 * NSEC_SEC;
			while (passes(bRead) <= previous && getTns() < deadline)
			{
				::usleep(1000);
			}
			assert(passes(bRead) > previous);
		}

		void waitPaused(bool bRead)
		{
			const uint64_t deadline = getTns() + 2 * NSEC_SEC;
			while (getTns() < deadline)
			{
				const size_t previous = passes(bRead);
				::usleep(30000);
				if (passes(bRead) == previous)
				{
					return;
				}
			}
			assert(false);
		}

		void assertSeparateWorkers(void)
		{
			assert(m_sendThread != 0 && m_readThread != 0);
			assert(!pthread_equal(m_sendThread.load(), m_readThread.load()));
		}

	private:
		std::atomic<size_t> m_nReads{0};
		std::atomic<size_t> m_nSends{0};
		std::atomic<pthread_t> m_sendThread{0};
		std::atomic<pthread_t> m_readThread{0};
	};

	class CANetHarness : public CANworkerHarness<_USR_CANET>
	{
	public:
		void setBytes(BytePacketStream *pIn, BytePacketStream *pOut)
		{
			m_pBpStreamIn = pIn;
			m_pBpStreamOut = pOut;
		}

		void attachStreams(CANframeStream &in, CANframeStream &out, BytePacketStream &bytesIn, BytePacketStream &bytesOut)
		{
			setStreams(&in, &out);
			setBytes(&bytesIn, &bytesOut);
		}
	};

	class SocketCANharness : public CANworkerHarness<_SocketCAN>
	{
	public:
		void attach(int fd)
		{
			m_socket = fd;
			m_bOpened = true;
		}

		void attachStreams(CANframeStream &in, CANframeStream &out, BytePacketStream &bytesIn, BytePacketStream &bytesOut)
		{
			setStreams(&in, &out);
		}
	};

	class UnopenedSocketCANharness : public SocketCANharness
	{
	public:
		bool open(void) override
		{
			return false;
		}
	};

	void configure(_ModuleBase &codec, JsonCfg &cfg)
	{
		const string className = dynamic_cast<_SocketCAN *>(&codec) ? "_SocketCAN" : "_USR_CANET";
		cfg.setJson({{"codec", {
			{"class", className},
			{"thread", {{"FPS", 500}}}, {"threadR", {{"FPS", 500}}}
		}}});
		codec.setName("codec");
		codec.setConfig(&cfg, &(*cfg.getJson())["codec"]);
		assert(codec.loadConfig());
		assert(codec.saveConfig(false));
		assert((*cfg.getJson())["codec"]["threadR"]["FPS"] == 500);
	}

	CAN_FRAME frame(uint32_t id, uint8_t length, uint64_t tStamp, bool bExtended = false, bool bRTR = false)
	{
		CAN_FRAME result;
		result.m_ID = id;
		result.m_nData = length;
		result.m_tStamp = tStamp;
		result.m_bExtended = bExtended;
		result.m_bRTR = bRTR;
		for (uint8_t i = 0; i < length; ++i)
		{
			result.m_pData[i] = i + 10;
		}
		return result;
	}

	void assertFrame(const CAN_FRAME &actual, const CAN_FRAME &expected)
	{
		assert(actual.m_ID == expected.m_ID);
		assert(actual.m_nData == expected.m_nData);
		assert(actual.m_bExtended == expected.m_bExtended);
		assert(actual.m_bRTR == expected.m_bRTR);
		assert(actual.m_tStamp > 0);
		assert(memcmp(actual.m_pData, expected.m_pData, expected.m_nData) == 0);
	}

	void append(vector<uint8_t> &bytes, const vector<uint8_t> &suffix)
	{
		bytes.insert(bytes.end(), suffix.begin(), suffix.end());
	}

	void testCANetFrames(void)
	{
		BytePacketStream bytesIn;
		BytePacketStream bytesOut;
		CANframeStream framesIn;
		CANframeStream framesOut;
		JsonCfg cfg;
		CANetHarness codec;
		configure(codec, cfg);
		codec.setStreams(&framesIn, &framesOut);
		codec.setBytes(&bytesIn, &bytesOut);

		const vector<CAN_FRAME> expected{
			frame(0x123, 2, 100), frame(0x1abcde, 8, 101, true), frame(0x456, 0, 102, false, true)
		};
		// Stream revision timestamps may be newer than individual frame timestamps.
		framesIn.add(expected, 10000);
		assert(codec.sendFrame());
		assert(!codec.sendFrame());
		vector<BYTE_PACKET> packets;
		bytesOut.getPackets(packets);
		assert(packets.size() == expected.size());
		assert(packets[0].m_vB == vector<uint8_t>({2, 0, 0, 1, 0x23, 10, 11, 0, 0, 0, 0, 0, 0}));
		assert(packets[1].m_vB == vector<uint8_t>({0x88, 0, 0x1a, 0xbc, 0xde, 10, 11, 12, 13, 14, 15, 16, 17}));
		assert(packets[2].m_vB == vector<uint8_t>({0x40, 0, 0, 4, 0x56, 0, 0, 0, 0, 0, 0, 0, 0}));

		const vector<uint8_t> &first = packets[0].m_vB;
		bytesIn.addPacket(vector<uint8_t>(first.begin(), first.begin() + 5));
		assert(!codec.readFrame());
		vector<uint8_t> tail(first.begin() + 5, first.end());
		append(tail, packets[1].m_vB);
		vector<uint8_t> malformed = first;
		malformed[0] = 9;
		append(tail, malformed);
		malformed[0] = 0x12;
		append(tail, malformed);
		malformed = first;
		malformed[1] = 0x20;
		append(tail, malformed);
		append(tail, packets[2].m_vB);
		tail.insert(tail.end(), first.begin(), first.begin() + 2);
		bytesIn.addPacket(tail);
		assert(codec.readFrame());
		assert(!codec.readFrame());
		vector<CAN_FRAME> decoded;
		framesOut.get(decoded);
		assert(decoded.size() == expected.size());
		for (size_t i = 0; i < decoded.size(); ++i)
		{
			assertFrame(decoded[i], expected[i]);
			if (i > 0)
			{
				assert(decoded[i].m_tStamp > decoded[i - 1].m_tStamp);
			}
		}
		const uint64_t lastStamp = decoded.back().m_tStamp;
		bytesIn.addPacket(vector<uint8_t>(first.begin() + 2, first.end()));
		assert(codec.readFrame());
		assert(!codec.readFrame());
		assert(!codec.readFrame());
		framesOut.get(decoded, lastStamp);
		assert(decoded.size() == 1);
		assertFrame(decoded[0], expected[0]);

		framesIn.add({frame(0x321, 9, 103), frame(0x800, 1, 104), frame(0x321, 1, 105)}, 10001);
		assert(codec.sendFrame());
		assert(!codec.sendFrame());
		bytesOut.getPackets(packets);
		assert(packets.size() == 4);
		assert(packets.back().m_vB[0] == 1 && packets.back().m_vB[3] == 3 && packets.back().m_vB[4] == 0x21);
	}

	void socketPair(int (&fd)[2])
	{
		assert(::socketpair(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0, fd) == 0);
	}

	void testSocketCANframes(void)
	{
		int fd[2];
		socketPair(fd);
		CANframeStream framesIn;
		CANframeStream framesOut;
		SocketCANharness codec;
		codec.attach(fd[0]);
		codec.setStreams(&framesIn, &framesOut);
		const vector<CAN_FRAME> expected{
			frame(0x123, 2, 100), frame(0x1abcde, 8, 101, true), frame(0x456, 0, 102, false, true)
		};
		framesIn.add(expected, 10000);
		assert(codec.sendFrame());
		assert(!codec.sendFrame());
		for (const CAN_FRAME &item : expected)
		{
			can_frame wire{};
			assert(::read(fd[1], &wire, sizeof(wire)) == sizeof(wire));
			const uint32_t flags = (item.m_bExtended ? CAN_EFF_FLAG : 0) | (item.m_bRTR ? CAN_RTR_FLAG : 0);
			assert(wire.can_id == (item.m_ID | flags));
			assert(wire.len == item.m_nData);
			assert(memcmp(wire.data, item.m_pData, item.m_nData) == 0);
			assert(::write(fd[1], &wire, sizeof(wire)) == sizeof(wire));
			assert(codec.readFrame());
		}
		assert(!codec.readFrame());
		vector<CAN_FRAME> decoded;
		framesOut.get(decoded);
		assert(decoded.size() == expected.size());
		for (size_t i = 0; i < decoded.size(); ++i)
		{
			assertFrame(decoded[i], expected[i]);
			if (i > 0)
			{
				assert(decoded[i].m_tStamp > decoded[i - 1].m_tStamp);
			}
		}

		can_frame malformed{};
		malformed.can_id = 0x321;
		assert(::write(fd[1], &malformed, sizeof(malformed) - 1) == sizeof(malformed) - 1);
		assert(!codec.readFrame());
		malformed.len = 9;
		assert(::write(fd[1], &malformed, sizeof(malformed)) == sizeof(malformed));
		assert(!codec.readFrame());
		malformed.len = 1;
		malformed.can_id = CAN_ERR_FLAG | 1;
		assert(::write(fd[1], &malformed, sizeof(malformed)) == sizeof(malformed));
		assert(!codec.readFrame());
		framesOut.get(decoded);
		assert(decoded.size() == 3);

		framesIn.add({frame(0x321, 9, 103), frame(0x800, 1, 104), frame(0x321, 1, 105)}, 10001);
		assert(codec.sendFrame());
		assert(!codec.sendFrame());
		can_frame wire{};
		assert(::read(fd[1], &wire, sizeof(wire)) == sizeof(wire));
		assert(wire.can_id == 0x321 && wire.len == 1 && wire.data[0] == 10);
		assert(::read(fd[1], &wire, sizeof(wire)) == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));
		::close(fd[1]);
	}

	void testSocketCANretry(void)
	{
		int fd[2];
		socketPair(fd);
		CANframeStream framesIn;
		SocketCANharness codec;
		codec.attach(fd[0]);
		codec.setStreams(&framesIn, nullptr);
		can_frame filler{};
		filler.can_id = 0x7ff;
		size_t filled = 0;
		while (::write(fd[0], &filler, sizeof(filler)) == sizeof(filler))
		{
			assert(++filled < 10000);
		}
		assert(filled > 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
		assert(::read(fd[1], &filler, sizeof(filler)) == sizeof(filler));
		assert(filler.can_id == 0x7ff);
		--filled;

		// Only one frame fits before EAGAIN. Both have the same timestamp, so
		// retaining the failed suffix is required after the cursor advances.
		framesIn.add({frame(0x101, 1, 200), frame(0x102, 1, 200)}, 10000);
		assert(!codec.sendFrame());
		framesIn.add({frame(0x103, 1, 201)}, 10001);
		for (size_t i = 0; i < filled; ++i)
		{
			assert(::read(fd[1], &filler, sizeof(filler)) == sizeof(filler));
			assert(filler.can_id == 0x7ff);
		}

		assert(codec.sendFrame());
		codec.sendFrame();
		assert(!codec.sendFrame());
		for (uint32_t id : {0x101, 0x102, 0x103})
		{
			can_frame wire{};
			assert(::read(fd[1], &wire, sizeof(wire)) == sizeof(wire));
			assert(wire.can_id == id && wire.len == 1 && wire.data[0] == 10);
		}
		assert(::read(fd[1], &filler, sizeof(filler)) == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));
		::close(fd[1]);
	}

	void testCANlinks(void)
	{
		JsonCfg cfg;
		cfg.setJson({
			{"framesIn", {{"class", "CANframeStream"}}},
			{"framesOut", {{"class", "CANframeStream"}}},
			{"bytesIn", {{"class", "BytePacketStream"}}},
			{"bytesOut", {{"class", "BytePacketStream"}}},
			{"codec", json::object()}
		});
		InstanceMgr instances;
		json &config = *cfg.getJson();
		for (const string name : {"framesIn", "framesOut", "bytesIn", "bytesOut"})
		{
			assert(instances.addDataObject(name, &cfg, &config[name]));
		}
		assert(instances.initAll());
		const json send{{"class", "_USR_CANET"}, {"CANframeStreamIn", "framesIn"}, {"BytePacketStreamOut", "bytesOut"}};
		const json receive{{"class", "_USR_CANET"}, {"CANframeStreamOut", "framesOut"}, {"BytePacketStreamIn", "bytesIn"}};
		for (const json &links : {send, receive})
		{
			config["codec"] = links;
			_USR_CANET codec;
			codec.setName("codec");
			codec.setConfig(&cfg, &config["codec"]);
			assert(codec.loadConfig());
			assert(codec.link(&instances));
			assert(codec.saveConfig(false));
			for (auto item = links.begin(); item != links.end(); ++item)
			{
				assert(config["codec"][item.key()] == item.value());
			}
		}
		config["codec"] = send;
		config["codec"]["CANframeStreamIn"] = "bytesIn";
		_USR_CANET invalid;
		invalid.setName("codec");
		invalid.setConfig(&cfg, &config["codec"]);
		assert(invalid.loadConfig());
		assert(!invalid.link(&instances));
		config["codec"] = {{"class", "_USR_CANET"}, {"CANframeStreamIn", "framesIn"}};
		assert(!invalid.link(&instances));

		for (const string key : {"CANframeStreamIn", "CANframeStreamOut"})
		{
			config["codec"] = {{"class", "_SocketCAN"}, {key, "framesIn"}};
			_SocketCAN codec;
			codec.setName("codec");
			codec.setConfig(&cfg, &config["codec"]);
			assert(codec.loadConfig());
			assert(codec.link(&instances));
			config["codec"][key] = "bytesIn";
			assert(!codec.link(&instances));
		}
	}

	template <class Codec>
	void assertStopped(Codec &codec)
	{
		const uint64_t started = getTns();
		codec.stop();
		assert(getTns() - started < NSEC_SEC);
		assert(codec.bStopped() && !codec.bRun());
		assert(codec.getThread("thread")->bStopped());
		assert(codec.getThread("threadR")->bStopped());
	}

	template <class Codec>
	void exerciseWorkers(Codec &codec)
	{
		assert(codec.start());
		codec.waitPasses(false, 0);
		codec.waitPasses(true, 0);
		codec.assertSeparateWorkers();

		codec.getThread("thread")->pause();
		codec.waitPaused(false);
		const size_t pausedSends = codec.passes(false);
		codec.waitPasses(true, codec.passes(true));
		assert(codec.passes(false) == pausedSends);
		codec.getThread("threadR")->pause();
		codec.waitPaused(true);
		const size_t pausedReads = codec.passes(true);
		codec.getThread("thread")->run();
		codec.waitPasses(false, pausedSends);
		assert(codec.passes(true) == pausedReads);

		codec.pause();
		codec.waitPaused(false);
		codec.waitPaused(true);
		const size_t sends = codec.passes(false);
		const size_t reads = codec.passes(true);
		codec.resume();
		codec.waitPasses(false, sends);
		codec.waitPasses(true, reads);
		assertStopped(codec);
	}

	template <class Codec>
	void testStartRollback(void)
	{
		for (const bool bRead : {false, true})
		{
			CANframeStream framesIn;
			CANframeStream framesOut;
			BytePacketStream bytesIn;
			BytePacketStream bytesOut;
			JsonCfg cfg;
			Codec codec;
			configure(codec, cfg);
			codec.attachStreams(framesIn, framesOut, bytesIn, bytesOut);
			FailingCANthread *pFailure = codec.failWorker(bRead);
			assert(!codec.start());
			assert(pFailure->m_bAttempted);
			assertStopped(codec);
		}
	}

	void testCANworkers(void)
	{
		BytePacketStream bytesIn;
		BytePacketStream bytesOut;
		CANframeStream framesIn;
		CANframeStream framesOut;
		JsonCfg canetCfg;
		CANetHarness canet;
		configure(canet, canetCfg);
		canet.setBytes(&bytesIn, &bytesOut);
		canet.setStreams(&framesIn, &framesOut);
		framesIn.add({frame(0x123, 2, 100)});
		bytesIn.addPacket({2, 0, 0, 1, 0x23, 10, 11, 0, 0, 0, 0, 0, 0});
		exerciseWorkers(canet);
		vector<BYTE_PACKET> packets;
		bytesOut.getPackets(packets);
		assert(packets.size() == 1);
		vector<CAN_FRAME> decoded;
		framesOut.get(decoded);
		assert(decoded.size() == 1);

		int fd[2];
		socketPair(fd);
		CANframeStream socketFramesOut;
		JsonCfg socketCfg;
		SocketCANharness socketCodec;
		configure(socketCodec, socketCfg);
		socketCodec.attach(fd[0]);
		socketCodec.setStreams(&framesIn, &socketFramesOut);
		can_frame wire{};
		wire.can_id = 0x456;
		assert(::write(fd[1], &wire, sizeof(wire)) == sizeof(wire));
		exerciseWorkers(socketCodec);
		assert(::read(fd[1], &wire, sizeof(wire)) == sizeof(wire));
		assert(wire.can_id == 0x123);
		socketFramesOut.get(decoded);
		assert(decoded.size() == 1 && decoded[0].m_ID == 0x456);
		::close(fd[1]);

		JsonCfg unopenedCfg;
		UnopenedSocketCANharness unopened;
		configure(unopened, unopenedCfg);
		unopened.setStreams(&framesIn, &framesOut);
		assert(unopened.start());
		::usleep(5000);
		assertStopped(unopened);
		testStartRollback<CANetHarness>();
		testStartRollback<UnopenedSocketCANharness>();
	}
}

void runCANStreamTests(void)
{
	testCANetFrames();
	testSocketCANframes();
	testSocketCANretry();
	testCANlinks();
	testCANworkers();
}

#endif
