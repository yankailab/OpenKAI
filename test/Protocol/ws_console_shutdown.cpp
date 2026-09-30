#include "Protocol/_WSconsole.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace kai;
using namespace std::chrono_literals;

namespace
{
    void require(bool condition, const char *message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    class MemoryIO : public _IObase
    {
    public:
        MemoryIO() { require(m_packetW.init(1, 1), "Cannot initialize test IO"); }
        bool bOpen() override { return connected; }
        int read(uint8_t *, int) override { ++reads; return 0; }
        bool write(uint8_t *, int) override { ++writes; return true; }
        std::atomic_bool connected{true};
        std::atomic_uint reads{0}, writes{0};
    };

    class FailedThread : public _Thread
    {
    public:
        bool startThread(void *(*)(void *), void *) override { return false; }
    };

    class ConsoleProbe : public _WSconsole
    {
    public:
        void attach(MemoryIO &io) { m_pIO = &io; }
        bool workersStopped() const { return m_pT->bStopped() && m_pTr->bStopped(); }
        void failReceiverStart()
        {
            delete m_pTr;
            m_pTr = new FailedThread;
        }
    };

    void waitForReads(MemoryIO &io)
    {
        for (int i = 0; i < 100 && io.reads == 0; ++i) std::this_thread::sleep_for(1ms);
        require(io.reads > 0, "Console receiver did not run");
    }

    void initialize(ConsoleProbe &console, MemoryIO &io, JsonCfg &owner, json &config)
    {
        console.setName("console");
        config["class"] = "_WSconsole";
        console.setConfig(&owner, &config);
        require(console.loadConfig(), "Cannot initialize console");
        console.attach(io);
    }

    void testStop(bool closeIO)
    {
        JsonCfg owner;
        json config = {{"thread", {{"FPS", 1000}}}, {"threadR", {{"FPS", 1000}}}};
        MemoryIO io;
        ConsoleProbe console;
        initialize(console, io, owner, config);
        require(console.start(), "Cannot start console");
        waitForReads(io);
        if (closeIO) io.connected = false;
        console.stop();
        require(console.workersStopped(), "stop() did not join both console workers");
        const auto reads = io.reads.load(), writes = io.writes.load();
        std::this_thread::sleep_for(5ms);
        require(io.reads == reads && io.writes == writes, "IO continued after stop() returned");
        console.stop();
        io.connected = true;
        require(console.start(), "Joined console workers could not restart");
        console.stop();
        require(console.workersStopped(), "Restarted console workers did not stop");
    }

    void testDestructor()
    {
        JsonCfg owner;
        json config = {{"thread", {{"FPS", 1000}}}, {"threadR", {{"FPS", 1000}}}};
        MemoryIO io;
        for (int i = 0; i < 20; ++i)
        {
            io.reads = 0;
            auto *console = new ConsoleProbe;
            initialize(*console, io, owner, config);
            require(console->start(), "Cannot start console for destruction test");
            waitForReads(io);
            delete console;
            const auto reads = io.reads.load(), writes = io.writes.load();
            std::this_thread::sleep_for(2ms);
            require(io.reads == reads && io.writes == writes, "Workers survived console destruction");
        }
    }

    void testPartialStartupFailure()
    {
        JsonCfg owner;
        json config = {{"thread", {{"FPS", 1000}}}, {"threadR", {{"FPS", 1000}}}};
        MemoryIO io;
        ConsoleProbe console;
        initialize(console, io, owner, config);
        console.failReceiverStart();
        require(!console.start(), "Receiver startup failure was ignored");
        require(console.workersStopped(), "Sender survived failed receiver startup");
    }
}

int main(int argc, char **argv)
{
    google::InitGoogleLogging(argv[0]);
    try
    {
        testStop(false);
        testStop(true);
        testDestructor();
        testPartialStartupFailure();
        std::cout << "PASS: console stop, closed IO, restart, destruction, and partial startup failure\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
