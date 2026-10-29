#ifdef USE_WSSERVER

#include "../../src/IO/_WebSocketServer.h"
#include <cassert>
#include <csignal>
#include <iostream>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <thread>

namespace
{
	using namespace kai;

	class WebSocketHarness : public _WebSocketServer
	{
	public:
		void attach(BytePacketStream &input, BytePacketStream &output)
		{
			m_pBpStreamIn = &input;
			m_pBpStreamOut = &output;
		}
	};

	static size_t packetCount(BytePacketStream &stream)
	{
		vector<BYTE_PACKET> packets;
		stream.get(packets);
		return packets.size();
	}

	static void sendText(int fd, const string &text)
	{
		assert(text.size() < 126);
		vector<uint8_t> bytes{0x81, static_cast<uint8_t>(0x80 | text.size()), 1, 2, 3, 4};
		for (size_t i = 0; i < text.size(); ++i)
		{
			bytes.push_back(static_cast<uint8_t>(text[i]) ^ bytes[2 + i % 4]);
		}
		assert(::send(fd, bytes.data(), bytes.size(), MSG_NOSIGNAL) == static_cast<ssize_t>(bytes.size()));
	}

	static int connectClient(const sockaddr_in &addr)
	{
		int fd = -1;
		const uint64_t deadline = getTns() + 2 * NSEC_SEC;
		while (getTns() < deadline)
		{
			fd = socket(AF_INET, SOCK_STREAM, 0);
			int bufSize = 4096;
			setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bufSize, sizeof(bufSize));
			if (connect(fd, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) == 0)
			{
				break;
			}
			close(fd);
			fd = -1;
			usleep(1000);
		}
		assert(fd >= 0);
		const string request = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
		assert(send(fd, request.data(), request.size(), MSG_NOSIGNAL) == static_cast<ssize_t>(request.size()));
		string response;
		while (response.find("\r\n\r\n") == string::npos)
		{
			char buf[512];
			ssize_t n = recv(fd, buf, sizeof(buf), 0);
			assert(n > 0);
			response.append(buf, n);
		}
		assert(response.find("101") != string::npos);
		return fd;
	}

	static void receiveReply(int fd, int mode, const string &expected)
	{
		pollfd ready{fd, POLLIN, 0};
		assert(poll(&ready, 1, 1000) > 0);
		uint8_t header[2];
		assert(recv(fd, header, 2, MSG_WAITALL) == 2);
		assert(header[0] == (mode < 2 ? 0x82 : 0x81));
		assert(header[1] == expected.size());
		string payload(expected.size(), '\0');
		assert(recv(fd, payload.data(), payload.size(), MSG_WAITALL) == static_cast<ssize_t>(payload.size()));
		assert(payload == expected);
	}

	void testWebSocketMode(int mode, bool readAll)
	{
		signal(SIGPIPE, SIG_IGN);
		int portFd = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		assert(bind(portFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0);
		socklen_t addrLen = sizeof(addr);
		assert(getsockname(portFd, reinterpret_cast<sockaddr *>(&addr), &addrLen) == 0);
		close(portFd);
		JsonCfg cfg;
		cfg.setJson({{"class", "_WebSocketServer"}, {"name", "duplex-ws"},
					 {"host", "127.0.0.1"}, {"port", ntohs(addr.sin_port)}, {"wsMode", mode}, {"tOutMs", 1500},
					 {"thread", {{"FPS", 500}}}, {"threadR", {{"FPS", 500}}}});
		if (readAll) (*cfg.getJson())["bReadAllClients"] = true;
		InstanceMgr manager;
		BytePacketStream input;
		BytePacketStream output;
		WebSocketHarness server;
		server.setConfig(&cfg, cfg.getJson());
		assert(server.loadConfig());
		assert(server.saveConfig(false));
		assert(cfg.getJson()->at("bReadAllClients") == readAll);
		assert(server.link(&manager));
		server.attach(input, output);
		assert(server.start());
		const int fd = connectClient(addr);
		uint64_t deadline = getTns() + NSEC_SEC;
		while (server.nClient() == 0 && getTns() < deadline)
		{
			usleep(1000);
		}
		assert(server.nClient() == 1);
		const int second = connectClient(addr);
		deadline = getTns() + NSEC_SEC;
		while (server.nClient() < 2 && getTns() < deadline) usleep(1000);
		assert(server.nClient() == 2);

		// Default routing stays first-client-only in every send mode. The opt-in
		// accepts complete commands from both readers without changing send mode.
		sendText(second, "secondaryEOJ");
		sendText(fd, "primaryEOJ");
		deadline = getTns() + NSEC_SEC;
		const size_t expectedCount = readAll ? 2 : 1;
		while (packetCount(output) < expectedCount && getTns() < deadline) usleep(1000);
		assert(packetCount(output) == expectedCount);
		vector<BYTE_PACKET> packets;
		output.get(packets);
		bool primary = false, secondary = false;
		for (const auto &packet : packets)
		{
			const string message(packet.m_vB.begin(), packet.m_vB.end());
			primary |= message == "primaryEOJ";
			secondary |= message == "secondaryEOJ";
		}
		assert(primary && secondary == readAll);
		output.clear();

		if (readAll)
		{
			// Concurrent callbacks must preserve complete packets and increasing
			// timestamps, because consumers advance a strict timestamp cursor.
			auto burst = [](int socket, char prefix) {
				for (int i = 0; i < 32; ++i) sendText(socket, string(1, prefix) + std::to_string(i) + "EOJ");
			};
			std::thread firstBurst(burst, fd, 'A'), secondBurst(burst, second, 'B');
			firstBurst.join(); secondBurst.join();
			deadline = getTns() + NSEC_SEC;
			while (packetCount(output) < 64 && getTns() < deadline) usleep(1000);
			output.get(packets);
			assert(packets.size() == 64);
			uint64_t previousStamp = 0;
			int nextA = 0, nextB = 0;
			for (const auto &packet : packets)
			{
				assert(packet.m_tStamp > previousStamp);
				previousStamp = packet.m_tStamp;
				const string message(packet.m_vB.begin(), packet.m_vB.end());
				int &sequence = message.front() == 'A' ? nextA : nextB;
				assert(message == string(1, message.front()) + std::to_string(sequence++) + "EOJ");
			}
			assert(nextA == 32 && nextB == 32);
			output.clear();
		}

		const string reply = "{\"cmd\":\"reply\"}";
		input.add({{vector<uint8_t>(reply.begin(), reply.end()), getTns()}});
		receiveReply(fd, mode, reply);
		if (mode == wsSocket_bin_bcast || mode == wsSocket_txt_bcast) receiveReply(second, mode, reply);
		else
		{
			pollfd ready{second, POLLIN, 0};
			assert(poll(&ready, 1, 80) == 0);
		}
		server.pause();
		sendText(fd, "paused");
		usleep(80000);
		assert(packetCount(output) == 0);
		server.resume();
		deadline = getTns() + NSEC_SEC;
		while (packetCount(output) == 0 && getTns() < deadline)
		{
			usleep(1000);
		}
		assert(packetCount(output) == 1);
		output.clear();
		input.add({{vector<uint8_t>(16 * 1024 * 1024, 'x'), getTns()}});
		pollfd pollFd{fd, POLLIN, 0};
		assert(poll(&pollFd, 1, 1000) > 0);
		usleep(50000);
		const uint64_t sent = getTns();
		sendText(fd, "duplex");
		deadline = sent + 300 * NSEC_MSEC;
		while (packetCount(output) == 0 && getTns() < deadline)
		{
			usleep(1000);
		}
		assert(packetCount(output) == 1);
		std::cout << "Received during blocked send after " << (getTns() - sent) / NSEC_MSEC << " ms\n";
		shutdown(fd, SHUT_RDWR);
		close(fd);
		shutdown(second, SHUT_RDWR);
		close(second);
		server.stop();
		assert(server.bStopped());
		assert(!server.bOpen());
		std::cout << "WebSocket duplex mode " << mode << ", read all clients " << readAll << " passed\n";
	}
}

namespace kai
{
	void runBytePacketWebSocketTests(void)
	{
		// wsServer keeps process-global socket state and has no listener close API.
		// Isolate each mode so its listening socket is released on child exit.
		for (int mode = 0; mode < 4; ++mode)
		for (bool readAll : {false, true})
		{
			std::cout.flush();
			pid_t child = fork();
			assert(child >= 0);
			if (child == 0)
			{
				alarm(8);
				testWebSocketMode(mode, readAll);
				std::cout.flush();
				_exit(0);
			}
			int status = 0;
			assert(waitpid(child, &status, 0) == child);
			assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
		}
	}
}

#endif
