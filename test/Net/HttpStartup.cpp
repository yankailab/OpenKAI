#include "../../src/Net/_HttpServer.h"
#include "../../src/UI/Viewer/Web/_WebGeometry.h"
#include "../../src/UI/Viewer/Web/_WebSelectableOctGrid.h"
#include "../../src/UI/Viewer/Web/_WebGLIM.h"
#include "../../src/UI/Viewer/Web/_WebMavlinkStream.h"
#undef timeout
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <unistd.h>

namespace
{
    using namespace kai;
    using namespace std::chrono_literals;
    namespace net = boost::asio;
    namespace http = boost::beast::http;
    using tcp = net::ip::tcp;

    void getIndex(uint16_t port)
    {
        net::io_context io;
        tcp::socket socket(io);
        const auto deadline = std::chrono::steady_clock::now() + 4s;
        for (;;)
        {
            boost::system::error_code ec;
            socket.connect({net::ip::make_address("127.0.0.1"), port}, ec);
            if (!ec) break;
            socket.close();
            assert(std::chrono::steady_clock::now() < deadline);
            std::this_thread::sleep_for(20ms);
        }
        http::request<http::empty_body> request{http::verb::get, "/", 11};
        request.set(http::field::host, "localhost");
        http::write(socket, request);
        boost::beast::flat_buffer buffer;
        http::response<http::string_body> response;
        http::read(socket, buffer, response);
        assert(response.result() == http::status::ok && response.body() == "retry test");
    }

    template<class Module>
    void retryTest(const char *name, const std::filesystem::path &root)
    {
        net::io_context io;
        tcp::acceptor occupied(io);
        occupied.open(tcp::v4());
        occupied.set_option(tcp::acceptor::reuse_address(true));
        occupied.bind({net::ip::make_address("127.0.0.1"), 0});
        occupied.listen();
        const auto port = occupied.local_endpoint().port();
        JsonCfg cfg;
        cfg.setJson({
            {"map", {{"class", "PCLmap"}}},
            {"vehicle", {{"class", "MavlinkStream"}}},
            {"server", {{"class", name}, {"host", "127.0.0.1"}, {"port", port},
                {"webRoot", root.string()}, {"modelsRoot", ""}, {"thread", {{"FPS", 20}}},
                {"PCLmapIn", "map"}, {"MavlinkStream", "vehicle"}}}});
        json &config = *cfg.getJson();
        InstanceMgr instances;
        assert(instances.addDataObject("map", &cfg, &config["map"]));
        assert(instances.addDataObject("vehicle", &cfg, &config["vehicle"]));
        assert(instances.initAll());
        Module module;
        module.setName("server");
        module.setConfig(&cfg, &config["server"]);
        assert(module.loadConfig() && module.link(&instances));

        const auto start = std::chrono::steady_clock::now();
        assert(module.start());
        assert(std::chrono::steady_clock::now() - start < 500ms);
        std::this_thread::sleep_for(1200ms); // More than one failed bind attempt.
        assert(module.bRun() && !module.bStopped());
        const auto stop = std::chrono::steady_clock::now();
        module.stop();
        assert(std::chrono::steady_clock::now() - stop < 500ms);
        assert(module.bStopped());

        // Restart while still occupied, then recover automatically after release.
        assert(module.start());
        std::this_thread::sleep_for(100ms);
        occupied.close();
        getIndex(port);
        module.stop();

        // A successful run must also leave a reusable module after stop().
        assert(module.start());
        getIndex(port);
        module.stop();
        assert(module.bStopped());
        config["server"]["port"] = -1;
        assert(!module.loadConfig());
        std::cout << name << " occupied-port retry, stop and restart passed\n";
    }
}

int main()
{
    const auto root = std::filesystem::temp_directory_path() /
        ("openkai-http-startup-" + std::to_string(getpid()));
    std::filesystem::create_directories(root / "vendor");
    std::ofstream(root / "index.html") << "retry test";
    std::ofstream(root / "vendor/three.module.min.js") << "// test fixture";
    retryTest<kai::_HttpServer>("_HttpServer", root);
    retryTest<kai::_WebGeometry>("_WebGeometry", root);
    retryTest<kai::_WebSelectableOctGrid>("_WebSelectableOctGrid", root);
    retryTest<kai::_WebGLIM>("_WebGLIM", root);
    retryTest<kai::_WebMavlinkStream>("_WebMavlinkStream", root);
    std::filesystem::remove_all(root);
}
