#include "IO/_ADIO_EBYTE.h"
#include "Instance/InstanceMgr.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

namespace
{
    using namespace kai;

    void require(bool condition, const string &message)
    {
        if (!condition) throw std::runtime_error(message);
    }

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
