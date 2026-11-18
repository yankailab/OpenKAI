#include "IO/_ADIO_EBYTE.h"
#include "Instance/InstanceMgr.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <future>
#include <poll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
    using namespace kai;

    void require(bool condition, const string &message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    template<class Predicate>
    void waitFor(Predicate predicate, const string &message)
    {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (!predicate() && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        require(predicate(), message);
    }

    // Real libmodbus TCP peer, restricted to an ephemeral loopback port.
    class TcpDevice
    {
    public:
        explicit TcpDevice(uint16_t value)
        {
            m_context = modbus_new_tcp("127.0.0.1", 0);
            require(m_context != nullptr, "Cannot create TCP peer");
            m_mapping = modbus_mapping_new(1, 1, 1, 1);
            require(m_mapping != nullptr, "Cannot create TCP mapping");
            m_mapping->tab_registers[0] = value;
            modbus_set_indication_timeout(m_context, 0, 100000);
            modbus_set_byte_timeout(m_context, 0, 100000);
            m_listener = modbus_tcp_listen(m_context, 4);
            require(m_listener >= 0, "Cannot listen on loopback: " + string(strerror(errno)));
            sockaddr_in address{};
            socklen_t size = sizeof(address);
            require(getsockname(m_listener, reinterpret_cast<sockaddr *>(&address), &size) == 0,
                    "Cannot get loopback port");
            port = ntohs(address.sin_port);
            m_thread = std::thread(&TcpDevice::run, this);
        }
        ~TcpDevice()
        {
            m_run = false;
            if (m_thread.joinable()) m_thread.join();
            ::close(m_listener);
            modbus_mapping_free(m_mapping);
            modbus_free(m_context);
        }
        int port = 0;
        std::atomic<int> connections{0};
        std::atomic<int> requests{0};
        std::atomic<bool> holdReply{false};

    private:
        void run()
        {
            int client = -1;
            uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
            while (m_run)
            {
                pollfd descriptor{client < 0 ? m_listener : client, POLLIN, 0};
                if (poll(&descriptor, 1, 20) <= 0) continue;
                if (client < 0)
                {
                    client = modbus_tcp_accept(m_context, &m_listener);
                    if (client >= 0) ++connections;
                    continue;
                }
                const int size = modbus_receive(m_context, query);
                if (size <= 0)
                {
                    modbus_close(m_context);
                    client = -1;
                    continue;
                }
                ++requests;
                while (m_run && holdReply)
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (m_run) modbus_reply(m_context, query, size, m_mapping);
            }
            if (client >= 0) modbus_close(m_context);
        }
        modbus_t *m_context = nullptr;
        modbus_mapping_t *m_mapping = nullptr;
        int m_listener = -1;
        std::atomic<bool> m_run{true};
        std::thread m_thread;
    };

    class SerialDevice
    {
    public:
        SerialDevice()
        {
            m_fd = posix_openpt(O_RDWR | O_NOCTTY | O_NONBLOCK);
            require(m_fd >= 0 && grantpt(m_fd) == 0 && unlockpt(m_fd) == 0,
                    "Cannot create pseudo-terminal");
            port = ptsname(m_fd);
        }
        ~SerialDevice() { ::close(m_fd); }
        void expectRequest(_Modbus &driver)
        {
            uint8_t request[] = {1, 3, 0, 0, 0, 1};
            // readRegisters waits for a reply before flushing. sendRawRequest
            // flushes immediately and may discard pending output on a PTY.
            auto reading = std::async(std::launch::async, [&] {
                uint16_t value = 0;
                return driver.readRegisters(1, 0, 1, &value);
            });
            pollfd descriptor{m_fd, POLLIN, 0};
            require(poll(&descriptor, 1, 500) == 1, "RTU request reached the wrong device");
            uint8_t received[8];
            require(::read(m_fd, received, sizeof(received)) == 8 &&
                    std::equal(request, request + sizeof(request), received), "Invalid RTU request");
            require(reading.get() == -1, "A silent RTU peer unexpectedly replied");
        }
        string port;
    private:
        int m_fd = -1;
    };

    struct ThreadObservation
    {
        bool destroyed = false;
        bool joinedBeforeDestruction = false;
    };

    class ObservedThread : public _Thread
    {
    public:
        ObservedThread(ThreadObservation &observation, bool controlledWorker)
            : m_observation(observation), m_controlledWorker(controlledWorker) {}
        ~ObservedThread() override
        {
            m_observation.destroyed = true;
            m_observation.joinedBeforeDestruction = joined();
            // Old drivers fail the assertion, but the test still owns cleanup.
            join();
        }
        bool joined() const { return m_threadID == 0; }
        bool startThread(void *(*worker)(void *), void *argument) override
        {
            if (!m_controlledWorker) return _Thread::startThread(worker, argument);
            return _Thread::startThread(waitForStop, this);
        }
    private:
        static void *waitForStop(void *argument)
        {
            auto *thread = static_cast<ObservedThread *>(argument);
            while (thread->bRun()) thread->sleepT(NSEC_SEC);
            return nullptr;
        }
        ThreadObservation &m_observation;
        bool m_controlledWorker;
    };

    template<class Driver>
    class DriverHarness : public Driver
    {
    public:
        DriverHarness(ThreadObservation &observation, bool controlledWorker = false)
            : m_observation(observation), m_controlledWorker(controlledWorker) {}
        ObservedThread *thread()
        {
            return static_cast<ObservedThread *>(this->getThread("thread"));
        }
    protected:
        _Thread *createThread(json *config, const string &name) override
        {
            if (!config) config = &((*this->m_pJ)[name] = json::object());
            auto thread = std::make_unique<ObservedThread>(m_observation, m_controlledWorker);
            thread->setName(name);
            thread->setConfig(this->m_pJcfg, config);
            if (!thread->loadConfig()) return nullptr;
            return thread.release();
        }
    private:
        ThreadObservation &m_observation;
        bool m_controlledWorker;
    };

    json driverConfig(const string &type, const string &port)
    {
        json config = {{"class", type}, {"thread", {{"FPS", 100}}}};
        if (type == "_Modbus")
            config.update({{"modbusType", "RTU"}, {"rtuPort", port},
                           {"rtuBaud", 115200}, {"rtuParity", "N"}});
        else
            config["ports"] = json::array();
        return config;
    }

    void configureTcp(_Modbus &driver, int port)
    {
        driver.setType("TCP");
        driver.setTcpAddr("127.0.0.1");
        driver.setTcpPort(port);
        driver.setResponseTimeout(0, 250000);
    }

    void expectRegister(_Modbus &driver, uint16_t expected)
    {
        uint16_t value = 0;
        require(driver.readRegisters(1, 0, 1, &value) == 1 && value == expected,
                "Read did not use the configured Modbus endpoint");
    }

    void reconfigure(const string &missingPort)
    {
        TcpDevice first(111), second(222);
        SerialDevice serialFirst, serialSecond;
        JsonCfg config;
        config.setJson({{"driver", driverConfig("_Modbus", missingPort)}});
        _Modbus driver;
        driver.setName("driver");
        driver.setConfig(&config, &(*config.getJson())["driver"]);
        require(driver.loadConfig(), "Initial configuration failed");
        configureTcp(driver, first.port);
        require(driver.start(), "Worker did not start");
        waitFor([&] { return driver.bOpen(); }, "Initial automatic connection failed");
        expectRegister(driver, 111);

        driver.close();
        driver.close();
        const int connections = first.connections;
        // Longer than several worker iterations: explicit close must stay closed.
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        require(!driver.bOpen() && first.connections == connections,
                "Worker reopened an explicitly closed connection");
        require(driver.writeRegister(1, 0, 7) == -1, "Closed bus accepted a transaction");
        configureTcp(driver, second.port);
        require(driver.open(), "New TCP endpoint did not open");
        expectRegister(driver, 222);
        require(driver.open(), "Repeated open failed");
        expectRegister(driver, 222);
        require(second.connections == 1, "Repeated open leaked an extra TCP connection");

        driver.close();
        driver.setType("RTU");
        driver.setRtuPort(serialFirst.port);
        driver.setRtuParity("N");
        driver.setRtuBaud(9600);
        driver.setRtuDataBits(8);
        driver.setRtuStopBits(1);
        require(driver.open(), "TCP to RTU switch failed");
        serialFirst.expectRequest(driver);
        driver.close();
        driver.setRtuPort(serialSecond.port);
        driver.setRtuBaud(19200);
        require(driver.open(), "New RTU device did not open");
        serialSecond.expectRequest(driver);
        driver.close();
        configureTcp(driver, first.port);
        require(driver.open(), "RTU to TCP switch failed");
        expectRegister(driver, 111);

        // Reloading JSON must preserve the running worker and explicit-close state.
        driver.close();
        auto &j = (*config.getJson())["driver"];
        j["modbusType"] = "TCP";
        j["tcpAddr"] = "127.0.0.1";
        j["tcpPort"] = second.port;
        require(driver.loadConfig(), "Running configuration reload failed");
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        require(!driver.bOpen(), "Reload reopened an explicitly closed bus");
        require(driver.open(), "Reloaded TCP connection failed");
        expectRegister(driver, 222);
        const int beforeReloadRetry = second.connections;
        uint16_t value = 0;
        require(driver.readRegisters(1, 99, 1, &value) == -1, "Expected error after reload");
        waitFor([&] { return second.connections > beforeReloadRetry && driver.bOpen(); },
                "Reload stopped the reconnect worker");
        expectRegister(driver, 222);
        driver.stop();
    }

    void configuration(const string &missingPort)
    {
        JsonCfg config;
        config.setJson({{"driver", driverConfig("_Modbus", missingPort)}});
        auto &j = (*config.getJson())["driver"];
        j.erase("modbusType");
        j["type"] = "RTU"; // Legacy, direct class loading only.
        _Modbus driver;
        driver.setName("driver");
        driver.setConfig(&config, &j);
        require(driver.loadConfig() && driver.getType() == "RTU", "Legacy transport was not loaded");
        require(driver.saveConfig(false) && j["modbusType"] == "RTU" && !j.contains("type"),
                "Save did not migrate the legacy transport key");
        driver.close();
        configureTcp(driver, 1502);
        driver.setRtuPort("/tmp/modbus-config-test");
        driver.setRtuParity("O");
        driver.setRtuBaud(38400);
        driver.setRtuDataBits(7);
        driver.setRtuStopBits(2);
        driver.setModbusDebug(true);
        driver.setErrReconnect(3);
        driver.setIntervalUsec(2345);
        driver.setResponseTimeout(2, 4567);
        j["type"] = "module";
        require(driver.saveConfig(false) && j["type"] == "module" && j["modbusType"] == "TCP",
                "Save corrupted the module discriminator");
        _Modbus restored;
        restored.setName("driver");
        restored.setConfig(&config, &j);
        require(restored.loadConfig(), "Cannot reload saved configuration");
        uint32_t sec = 0, usec = 0;
        restored.getResponseTimeout(sec, usec);
        require(restored.getType() == "TCP" && restored.getTcpAddr() == "127.0.0.1" &&
                restored.getTcpPort() == 1502 && restored.getRtuPort() == "/tmp/modbus-config-test" &&
                restored.getRtuParity() == "O" && restored.getRtuBaud() == 38400 &&
                restored.getRtuDataBits() == 7 && restored.getRtuStopBits() == 2 &&
                restored.getModbusDebug() && restored.getErrReconnect() == 3 &&
                restored.getIntervalUsec() == 2345 && sec == 2 && usec == 4567,
                "Connection settings did not round-trip");
        j["type"] = "RTU";
        require(restored.loadConfig() && restored.getType() == "TCP", "Legacy key overrode modbusType");

        // Invalid settings must fail safely, without leaving an open context.
        restored.setType("unsupported");
        require(!restored.open() && !restored.bOpen(), "Invalid type was accepted");
        restored.setType("RTU");
        restored.setRtuParity("");
        require(!restored.open(), "Empty parity was accepted");
        restored.setRtuParity("N");
        restored.setRtuDataBits(9);
        require(!restored.open(), "Invalid data bits were accepted");
        restored.setRtuDataBits(8);
        restored.setRtuStopBits(0);
        require(!restored.open(), "Invalid stop bits were accepted");
        restored.setRtuStopBits(1);
        restored.setResponseTimeout(0, 1000000);
        require(!restored.open(), "Invalid timeout was accepted");
        restored.setResponseTimeout(0, 0);
        require(!restored.open(), "Zero timeout was accepted");
        restored.setResponseTimeout(0, 50000);
        restored.setType("TCP");
        restored.setTcpPort(65536);
        require(!restored.open(), "Invalid TCP port was accepted");
        restored.close();
    }

    void concurrentClose(const string &missingPort)
    {
        TcpDevice peer(333);
        JsonCfg config;
        config.setJson({{"driver", driverConfig("_Modbus", missingPort)}});
        _Modbus driver;
        driver.setName("driver");
        driver.setConfig(&config, &(*config.getJson())["driver"]);
        require(driver.loadConfig(), "Cannot configure concurrent driver");
        configureTcp(driver, peer.port);
        require(driver.open() && driver.start(), "Cannot start concurrent driver");

        peer.holdReply = true;
        auto reading = std::async(std::launch::async, [&] {
            uint16_t value = 0;
            return driver.readRegisters(1, 0, 1, &value);
        });
        waitFor([&] { return peer.requests > 0; }, "Transaction never reached the peer");
        auto closing = std::async(std::launch::async, [&] { driver.close(); });
        require(closing.wait_for(std::chrono::milliseconds(30)) == std::future_status::timeout,
                "close() did not wait for the active transaction");
        peer.holdReply = false;
        require(reading.get() == 1, "close() interrupted an active transaction");
        closing.get();
        require(!driver.bOpen(), "Concurrent close left the bus open");

        // Exercise I/O and status/settings reads while the caller swaps contexts.
        std::atomic<bool> run{true};
        auto polling = std::async(std::launch::async, [&] {
            while (run)
            {
                uint16_t value = 0;
                driver.readRegisters(1, 0, 1, &value);
                driver.bOpen();
                driver.getTcpPort();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
        bool opened = true;
        for (int i = 0; i < 20; ++i)
        {
            driver.close();
            driver.setTcpPort(peer.port);
            opened = driver.open() && opened;
        }
        run = false;
        polling.get();
        require(opened, "Concurrent reconfiguration failed");
        driver.stop();
    }

    void reconnect(const string &missingPort)
    {
        TcpDevice peer(444);
        SerialDevice serial;
        JsonCfg config;
        config.setJson({{"driver", driverConfig("_Modbus", missingPort)}});
        _Modbus driver;
        driver.setName("driver");
        driver.setConfig(&config, &(*config.getJson())["driver"]);
        require(driver.loadConfig(), "Cannot configure disconnected driver");
        require(!driver.open(), "Missing serial device unexpectedly opened");
        require(driver.start(), "Cannot start disconnected driver");
        // A failed explicit open must still allow the worker to retry.
        driver.setRtuPort(serial.port);
        waitFor([&] { return driver.bOpen(); }, "Worker did not retry a failed explicit open");
        driver.close();
        configureTcp(driver, peer.port);
        require(driver.open(), "Cannot connect retry driver");
        expectRegister(driver, 444);
        // A Modbus exception counts as a transaction error and must trigger retry.
        uint16_t value = 0;
        require(driver.readRegisters(1, 99, 1, &value) == -1, "Expected illegal-address error");
        waitFor([&] { return peer.connections >= 2 && driver.bOpen(); }, "Automatic reconnect failed");
        expectRegister(driver, 444);
        driver.close();
        const int connections = peer.connections;
        std::this_thread::sleep_for(std::chrono::milliseconds(1100));
        require(!driver.bOpen() && peer.connections == connections,
                "Explicit close did not cancel automatic retries");
        driver.stop();
    }

    template<class Driver>
    void stoppedDriver(const string &type, const string &port)
    {
        JsonCfg config;
        config.setJson({{"driver", driverConfig(type, port)}});
        ThreadObservation observation;
        DriverHarness<Driver> driver(observation);
        driver.setName("driver");
        driver.setConfig(&config, &(*config.getJson())["driver"]);
        require(driver.loadConfig(), type + " loadConfig failed");
        require(!driver.check(), type + " unexpectedly has a connected device");
        driver.stop(); // Safe before start, even with a failed hardware check.
        for (unsigned pass = 0; pass < 3; ++pass)
        {
            require(driver.start(), type + " could not start/restart");
            if (pass == 0) std::this_thread::sleep_for(std::chrono::milliseconds(20));
            driver.stop();
            const bool stopped = !driver.thread()->bRun() && driver.thread()->joined();
            driver.thread()->join(); // Clean the old implementation before reporting failure.
            require(stopped, type + " stop() did not join its disconnected worker");
            driver.stop();
            require(driver.thread()->joined(), type + " repeated stop was not idempotent");
        }
    }

    template<class Driver>
    void destroyedDriver(const string &type, const string &port)
    {
        JsonCfg config;
        config.setJson({{"driver", driverConfig(type, port)}});
        ThreadObservation observation;
        // A cooperative test worker isolates destruction ordering from I/O.
        // Its late fallback join cannot race accesses to already-destroyed driver members.
        auto driver = std::make_unique<DriverHarness<Driver>>(observation, true);
        driver->setName("driver");
        driver->setConfig(&config, &(*config.getJson())["driver"]);
        require(driver->loadConfig(), type + " loadConfig failed");
        require(driver->start(), type + " start failed");
        driver.reset(); // Deliberately no explicit stop().
        require(observation.destroyed, type + " did not release its thread");
        require(observation.joinedBeforeDestruction,
                type + " destructor reached base thread destruction before joining its worker");
    }

    void managerLifecycle(const string &path, const string &port, bool busFirst)
    {
        const string bus = busFirst ? "a_bus" : "z_bus";
        const string io = busFirst ? "z_ebyte" : "a_ebyte";
        json config = {{"APP", {{"class", "InstanceMgr"}}},
                       {bus, driverConfig("_Modbus", port)}, {io, driverConfig("_ADIO_EBYTE", port)}};
        config[io]["_Modbus"] = bus;
        { std::ofstream output(path); output << config.dump(); }
        for (unsigned pass = 0; pass < 10; ++pass)
        {
            InstanceMgr manager;
            require(manager.loadJsonFiles(path) && manager.createAll() && manager.initAll() &&
                    manager.linkAll() && manager.startAll(), "Native manager startup failed");
            if (pass == 0) std::this_thread::sleep_for(std::chrono::milliseconds(20));
            // Remaining passes exercise start immediately followed by stop.
            manager.stopAll();
            manager.stopAll();
            manager.cleanAll();
            manager.cleanAll();
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) { std::cerr << "Usage: modbus-lifecycle <case> <temporary-directory>\n"; return 2; }
    const string test = argv[1], directory = argv[2];
    const string port = directory + "/nonexistent-serial-port";
    try
    {
        if (test == "stop-modbus") stoppedDriver<kai::_Modbus>("_Modbus", port);
        else if (test == "stop-ebyte") stoppedDriver<kai::_ADIO_EBYTE>("_ADIO_EBYTE", port);
        else if (test == "destroy-modbus") destroyedDriver<kai::_Modbus>("_Modbus", port);
        else if (test == "destroy-ebyte") destroyedDriver<kai::_ADIO_EBYTE>("_ADIO_EBYTE", port);
        else if (test == "manager-bus-first") managerLifecycle(directory + "/config.json", port, true);
        else if (test == "manager-ebyte-first") managerLifecycle(directory + "/config.json", port, false);
        else if (test == "reconfigure") reconfigure(port);
        else if (test == "configuration") configuration(port);
        else if (test == "concurrent-close") concurrentClose(port);
        else if (test == "reconnect") reconnect(port);
        else throw std::runtime_error("Unknown test: " + test);
        std::cout << "PASS: " << test << '\n';
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "FAIL: " << test << ": " << error.what() << '\n';
        return 1;
    }
}
