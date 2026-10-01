#include "../../src/IO/_IObase.h"

// Keep single-pass checks alongside tests that start both transport workers.
#define private public
#include "../../src/IO/_TCPclient.h"
#include "../../src/IO/_UDP.h"
#include "../../src/IO/_SerialPort.h"
#undef private

#include <atomic>
#include <cassert>
#include <poll.h>

namespace
{
	using namespace kai;

	class FailingThread : public _Thread
	{
	public:
		bool startThread(void *(*pStart)(void *), void *pArg) override
		{
			// pthread_create failure leaves a requested run state to clean up.
			m_setState = thread_run;
			return false;
		}
	};

	class LifecycleHarness : public _IObase
	{
	public:
		using _IObase::getThread;

		void failWorker(bool bRead)
		{
			_Thread *&pThread = bRead ? m_pTr : m_pT;
			delete pThread;
			pThread = new FailingThread();
			pThread->setName(bRead ? "threadR" : "thread");
		}
	};

	template <class Transport>
	class DuplexTransportHarness : public Transport
	{
	public:
		~DuplexTransportHarness()
		{
			this->stop();
		}

		using Transport::getThread;

		void readPackets(void) override
		{
			Transport::readPackets();
			++m_nReads;
		}

		void writePackets(void) override
		{
			Transport::writePackets();
			++m_nWrites;
		}

		size_t readPasses(void) const
		{
			return m_nReads.load();
		}

		size_t writePasses(void) const
		{
			return m_nWrites.load();
		}

		void waitPasses(bool bRead, size_t previous)
		{
			uint64_t deadline = getTns() + 2 * NSEC_SEC;
			while ((bRead ? readPasses() : writePasses()) <= previous && getTns() < deadline)
			{
				::usleep(1000);
			}
			assert((bRead ? readPasses() : writePasses()) > previous);
		}

		void pauseWorker(bool bRead)
		{
			getThread(bRead ? "threadR" : "thread")->pause();
			uint64_t deadline = getTns() + 2 * NSEC_SEC;
			while (getTns() < deadline)
			{
				size_t previous = bRead ? readPasses() : writePasses();
				::usleep(30000);
				if ((bRead ? readPasses() : writePasses()) == previous)
				{
					return;
				}
			}
			assert(false);
		}

	private:
		std::atomic<size_t> m_nReads{0};
		std::atomic<size_t> m_nWrites{0};
	};

	class TCPTransportHarness : public DuplexTransportHarness<_TCPclient>
	{
	public:
		void attach(int fd, BytePacketStream *pIn, BytePacketStream *pOut)
		{
			m_socket = fd;
			m_pBpStreamIn = pIn;
			m_pBpStreamOut = pOut;
			m_ioStatus = io_opened;
		}

		bool hasPartialWrite(void)
		{
			return m_iWrite > 0 && m_iWrite < m_bpWrite.m_vB.size();
		}
	};

	class UDPTransportHarness : public DuplexTransportHarness<_UDP>
	{
	public:
		void attach(int fd, BytePacketStream *pIn, BytePacketStream *pOut)
		{
			m_socket = fd;
			m_pBpStreamIn = pIn;
			m_pBpStreamOut = pOut;
			m_ioStatus = io_opened;
		}

		void setPeer(const sockaddr_in &peer)
		{
			m_sAddrRemote = peer;
		}
	};

	class SerialTransportHarness : public DuplexTransportHarness<_SerialPort>
	{
	public:
		void attach(const string &port, BytePacketStream *pIn, BytePacketStream *pOut)
		{
			m_port = port;
			m_pBpStreamIn = pIn;
			m_pBpStreamOut = pOut;
		}

		bool hasPartialWrite(void)
		{
			return m_iWrite > 0 && m_iWrite < m_bpWrite.m_vB.size();
		}
	};

	template <class Transport>
	void configureWorkers(Transport &transport, JsonCfg &cfg, const string &className)
	{
		cfg.setJson({{"class", className}, {"name", "duplex-test"},
					 {"thread", {{"FPS", 250}}}, {"threadR", {{"FPS", 500}}}});
		transport.setConfig(&cfg, cfg.getJson());
		assert(transport.loadConfig());
		_Thread *pWrite = transport.getThread("thread");
		_Thread *pRead = transport.getThread("threadR");
		assert(pWrite && pRead && pWrite != pRead);
		assert(pWrite->getTargetFPS() == 250);
		assert(pRead->getTargetFPS() == 500);
		assert(transport.saveConfig(false));
		assert((*cfg.getJson())["thread"]["FPS"] == 250);
		assert((*cfg.getJson())["threadR"]["FPS"] == 500);
	}

	vector<uint8_t> payload(size_t n)
	{
		vector<uint8_t> bytes(n);
		for (size_t i = 0; i < n; ++i)
		{
			bytes[i] = static_cast<uint8_t>(i % 251);
		}
		return bytes;
	}

	void appendAvailable(int fd, vector<uint8_t> &bytes)
	{
		uint8_t buf[8192];
		ssize_t n;
		while ((n = ::read(fd, buf, sizeof(buf))) > 0)
		{
			bytes.insert(bytes.end(), buf, buf + n);
		}
		assert(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
	}

	vector<uint8_t> streamBytes(BytePacketStream &stream)
	{
		vector<BYTE_PACKET> packets;
		stream.getPackets(packets);
		vector<uint8_t> bytes;
		for (const BYTE_PACKET &packet : packets)
		{
			bytes.insert(bytes.end(), packet.m_vB.begin(), packet.m_vB.end());
		}
		return bytes;
	}

	int boundDatagram(sockaddr_in &addr)
	{
		int fd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, IPPROTO_UDP);
		assert(fd >= 0);
		addr = {};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		assert(::bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0);
		socklen_t size = sizeof(addr);
		assert(::getsockname(fd, reinterpret_cast<sockaddr *>(&addr), &size) == 0);
		return fd;
	}

	void waitStreamBytes(BytePacketStream &stream, const vector<uint8_t> &expected)
	{
		uint64_t deadline = getTns() + 2 * NSEC_SEC;
		while (streamBytes(stream) != expected && getTns() < deadline)
		{
			::usleep(1000);
		}
		assert(streamBytes(stream) == expected);
	}

	void waitPeerBytes(int fd, const vector<uint8_t> &expected)
	{
		vector<uint8_t> received;
		uint64_t deadline = getTns() + 3 * NSEC_SEC;
		while (received.size() < expected.size() && getTns() < deadline)
		{
			appendAvailable(fd, received);
			::usleep(1000);
		}
		assert(received == expected);
	}

	template <class Transport>
	void stopWorkers(Transport &transport)
	{
		uint64_t started = getTns();
		transport.stop();
		assert(getTns() - started < NSEC_SEC);
		assert(transport.bStopped());
		assert(!transport.bRun());
		assert(transport.getThread("thread")->bStopped());
		assert(transport.getThread("threadR")->bStopped());
	}

	template <class Transport>
	void exerciseDuplexWorkers(Transport &transport, int peerFd,
							   BytePacketStream &in, BytePacketStream &out, bool bBackpressure)
	{
		assert(transport.start());
		transport.waitPasses(true, 0);
		transport.waitPasses(false, 0);

		transport.pauseWorker(false);
		size_t pausedWrites = transport.writePasses();
		vector<uint8_t> sent = payload(1000);
		vector<uint8_t> received = payload(750);
		in.addPacket(sent);
		assert(::write(peerFd, received.data(), received.size()) == static_cast<ssize_t>(received.size()));
		waitStreamBytes(out, received);
		assert(transport.writePasses() == pausedWrites);
		vector<uint8_t> unexpected;
		appendAvailable(peerFd, unexpected);
		assert(unexpected.empty());

		transport.pauseWorker(true);
		size_t pausedReads = transport.readPasses();
		vector<uint8_t> suffix{99, 98, 97};
		assert(::write(peerFd, suffix.data(), suffix.size()) == static_cast<ssize_t>(suffix.size()));
		transport.getThread("thread")->run();
		waitPeerBytes(peerFd, sent);
		assert(transport.readPasses() == pausedReads);
		assert(streamBytes(out) == received);
		transport.getThread("threadR")->run();
		received.insert(received.end(), suffix.begin(), suffix.end());
		waitStreamBytes(out, received);

		if (bBackpressure)
		{
			// Leave a payload larger than the socket/PTY buffers pending on TX.
			sent = payload(256 * 1024);
			in.addPacket(sent);
			pollfd descriptor{peerFd, POLLIN, 0};
			assert(::poll(&descriptor, 1, 2000) == 1);
			size_t previousWrites = transport.writePasses();
			transport.waitPasses(false, previousWrites + 2);
			assert(::write(peerFd, suffix.data(), suffix.size()) == static_cast<ssize_t>(suffix.size()));
			received.insert(received.end(), suffix.begin(), suffix.end());
			waitStreamBytes(out, received);
			waitPeerBytes(peerFd, sent);
		}

		transport.pause();
		transport.pauseWorker(true);
		transport.pauseWorker(false);
		size_t previousReads = transport.readPasses();
		size_t previousWrites = transport.writePasses();
		transport.resume();
		transport.waitPasses(true, previousReads);
		transport.waitPasses(false, previousWrites);

		// Joining both paused workers must wake them without another I/O pass.
		transport.pauseWorker(true);
		transport.pauseWorker(false);
		previousReads = transport.readPasses();
		previousWrites = transport.writePasses();
		stopWorkers(transport);
		assert(transport.readPasses() == previousReads);
		assert(transport.writePasses() == previousWrites);
	}

	template <class Transport>
	void exerciseSingleDirection(Transport &transport, int peerFd,
								 BytePacketStream &stream, bool bRead)
	{
		assert(transport.start());
		assert(transport.getThread("thread")->bRun());
		assert(transport.getThread("threadR")->bRun());
		vector<uint8_t> bytes = payload(750);
		if (bRead)
		{
			assert(::write(peerFd, bytes.data(), bytes.size()) == static_cast<ssize_t>(bytes.size()));
			waitStreamBytes(stream, bytes);
			assert(transport.writePasses() == 0);
		}
		else
		{
			stream.addPacket(bytes);
			waitPeerBytes(peerFd, bytes);
			assert(transport.readPasses() == 0);
		}
		stopWorkers(transport);
	}

	void testThreadConfigurationAndStartup(void)
	{
		JsonCfg cfg;
		cfg.setJson({{"class", "_IObase"}, {"name", "legacy-thread-test"},
					 {"thread", {{"FPS", 77}}}});
		LifecycleHarness transport;
		transport.setConfig(&cfg, cfg.getJson());
		assert(transport.loadConfig());
		assert(transport.getThread("thread")->getTargetFPS() == 77);
		assert(transport.getThread("threadR")->getTargetFPS() == 77);
		assert(transport.saveConfig(false));
		assert((*cfg.getJson())["threadR"]["FPS"] == 77);

		for (bool bRead : {false, true})
		{
			assert(transport.loadConfig());
			transport.failWorker(bRead);
			assert(!transport.start());
			assert(transport.bStopped());
			assert(!transport.bRun());
			assert(!transport.getThread("thread")->bRun());
			assert(!transport.getThread("threadR")->bRun());
		}
	}

	void testThreadedTCP(void)
	{
		for (int mode = 0; mode < 3; ++mode)
		{
			int sockets[2];
			assert(::socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets) == 0);
			int nSendBuf = 4096;
			assert(::setsockopt(sockets[0], SOL_SOCKET, SO_SNDBUF, &nSendBuf, sizeof(nSendBuf)) == 0);
			JsonCfg cfg;
			BytePacketStream in;
			BytePacketStream out;
			TCPTransportHarness transport;
			configureWorkers(transport, cfg, "_TCPclient");
			transport.attach(sockets[0], mode == 2 ? nullptr : &in, mode == 1 ? nullptr : &out);
			if (mode == 0)
			{
				exerciseDuplexWorkers(transport, sockets[1], in, out, true);
			}
			else
			{
				exerciseSingleDirection(transport, sockets[1], mode == 2 ? out : in, mode == 2);
			}
			::close(sockets[1]);
		}
	}

	void testThreadedUDP(void)
	{
		for (int mode = 0; mode < 3; ++mode)
		{
			sockaddr_in local;
			sockaddr_in peer;
			int fd = boundDatagram(local);
			int peerFd = boundDatagram(peer);
			assert(::connect(peerFd, reinterpret_cast<sockaddr *>(&local), sizeof(local)) == 0);
			JsonCfg cfg;
			BytePacketStream in;
			BytePacketStream out;
			UDPTransportHarness transport;
			configureWorkers(transport, cfg, "_UDP");
			transport.attach(fd, mode == 2 ? nullptr : &in, mode == 1 ? nullptr : &out);
			transport.setPeer(peer);
			if (mode == 0)
			{
				exerciseDuplexWorkers(transport, peerFd, in, out, false);
			}
			else
			{
				exerciseSingleDirection(transport, peerFd, mode == 2 ? out : in, mode == 2);
			}
			::close(peerFd);
		}
	}

	void testThreadedSerial(void)
	{
		for (int mode = 0; mode < 3; ++mode)
		{
			int master = ::posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
			assert(master >= 0);
			assert(::grantpt(master) == 0);
			assert(::unlockpt(master) == 0);
			char *pSlave = ::ptsname(master);
			assert(pSlave);
			JsonCfg cfg;
			BytePacketStream in;
			BytePacketStream out;
			SerialTransportHarness transport;
			configureWorkers(transport, cfg, "_SerialPort");
			transport.attach(pSlave, mode == 2 ? nullptr : &in, mode == 1 ? nullptr : &out);
			assert(transport.open());
			if (mode == 0)
			{
				exerciseDuplexWorkers(transport, master, in, out, true);
			}
			else
			{
				exerciseSingleDirection(transport, master, mode == 2 ? out : in, mode == 2);
			}
			::close(master);
		}
	}

	void testTCP(void)
	{
		int sockets[2];
		assert(::socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, sockets) == 0);
		int nSendBuf = 4096;
		assert(::setsockopt(sockets[0], SOL_SOCKET, SO_SNDBUF, &nSendBuf, sizeof(nSendBuf)) == 0);
		BytePacketStream in;
		BytePacketStream out;
		TCPTransportHarness transport;
		transport.attach(sockets[0], &in, &out);

		vector<uint8_t> received = payload(1500);
		assert(::write(sockets[1], received.data(), received.size()) == static_cast<ssize_t>(received.size()));
		transport.readPackets();
		assert(streamBytes(out) == received);

		vector<uint8_t> expected = payload(256 * 1024);
		in.addPacket(expected);
		transport.writePackets();
		assert(transport.hasPartialWrite());
		vector<uint8_t> suffix{99, 98, 97};
		in.addPacket(suffix);
		expected.insert(expected.end(), suffix.begin(), suffix.end());

		received.clear();
		for (int i = 0; i < 10000 && received.size() < expected.size(); ++i)
		{
			appendAvailable(sockets[1], received);
			transport.writePackets();
		}
		appendAvailable(sockets[1], received);
		assert(received == expected);
		transport.writePackets();
		appendAvailable(sockets[1], received);
		assert(received == expected);

		::close(sockets[1]);
		transport.readPackets();
		assert(!transport.bOpen());
	}

	void testUDP(void)
	{
		sockaddr_in local;
		sockaddr_in peer;
		int fd = boundDatagram(local);
		int peerFd = boundDatagram(peer);
		BytePacketStream in;
		BytePacketStream out;
		UDPTransportHarness transport;
		transport.attach(fd, &in, &out);

		vector<uint8_t> first = payload(4096);
		vector<uint8_t> second{3, 2, 1};
		assert(::sendto(peerFd, first.data(), first.size(), 0,
						reinterpret_cast<sockaddr *>(&local), sizeof(local)) == static_cast<ssize_t>(first.size()));
		assert(::sendto(peerFd, second.data(), second.size(), 0,
						reinterpret_cast<sockaddr *>(&local), sizeof(local)) == static_cast<ssize_t>(second.size()));
		transport.readPackets();
		vector<BYTE_PACKET> packets;
		out.getPackets(packets);
		assert(packets.size() == 2);
		assert(packets[0].m_vB == first && packets[1].m_vB == second);

		in.addPacket(first);
		in.addPacket(second);
		transport.writePackets();
		uint8_t buf[8192];
		ssize_t n = ::recv(peerFd, buf, sizeof(buf), 0);
		assert(n == static_cast<ssize_t>(first.size()));
		assert(vector<uint8_t>(buf, buf + n) == first);
		n = ::recv(peerFd, buf, sizeof(buf), 0);
		assert(n == static_cast<ssize_t>(second.size()));
		assert(vector<uint8_t>(buf, buf + n) == second);
		transport.writePackets();
		assert(::recv(peerFd, buf, sizeof(buf), 0) < 0);
		assert(errno == EAGAIN || errno == EWOULDBLOCK);
		::close(peerFd);
	}

	void testSerial(void)
	{
		int master = ::posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
		assert(master >= 0);
		assert(::grantpt(master) == 0);
		assert(::unlockpt(master) == 0);
		char *pSlave = ::ptsname(master);
		assert(pSlave);
		BytePacketStream in;
		BytePacketStream out;
		SerialTransportHarness transport;
		transport.attach(pSlave, &in, &out);
		assert(transport.open());

		vector<uint8_t> expected = payload(1500);
		assert(::write(master, expected.data(), expected.size()) == static_cast<ssize_t>(expected.size()));
		for (int i = 0; i < 100 && streamBytes(out).size() < expected.size(); ++i)
		{
			transport.readPackets();
			::usleep(1000);
		}
		assert(streamBytes(out) == expected);

		expected = payload(128 * 1024);
		in.addPacket(expected);
		transport.writePackets();
		assert(transport.hasPartialWrite());
		vector<uint8_t> received;
		for (int i = 0; i < 10000 && received.size() < expected.size(); ++i)
		{
			appendAvailable(master, received);
			transport.writePackets();
		}
		appendAvailable(master, received);
		assert(received == expected);
		transport.writePackets();
		appendAvailable(master, received);
		assert(received == expected);
		::close(master);
	}
}

namespace kai
{
	void runBytePacketTransportTests(void)
	{
		testTCP();
		testUDP();
		testSerial();
		testThreadConfigurationAndStartup();
		testThreadedTCP();
		testThreadedUDP();
		testThreadedSerial();
	}
}
