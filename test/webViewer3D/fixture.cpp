#include "../../src/Net/HttpServer.h"
#include "../../src/IO/WebSocketStream.h"
#include "../../src/3D/Viewer/WebViewer3DProtocol.h"
#include <atomic>
#include <cmath>
#include <iostream>
#include <thread>

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 4) return 1;
    std::vector<uint8_t> empty;
    float emptyBounds[6] = {};
    kai::webviewer3d::begin(empty, 0, 0);
    kai::webviewer3d::object(empty, 0, 2, 1, emptyBounds, {}, {}, {}, {});
    kai::webviewer3d::finish(empty, 1);
    if (empty.size() != 96) return 3;
    kai::HttpServer http;
    kai::WebSocketStream stream(http.context(), R"({"type":"hello","version":1,"autoBound":true,"showGrid":true,"background":[0.035,0.045,0.065],"camera":{"eye":[0,-8,4],"target":[0,0,0],"up":[0,0,1],"type":0,"fov":70,"near":0.01,"far":1000000,"lr":[-10,10],"bt":[-10,10]},"objects":[{"id":7,"name":"Test cloud and lines"}]})");
    std::string error;
    if (!http.start("127.0.0.1", 0, argv[1], stream.upgradeHandler(), &error))
    { std::cerr << error << std::endl; return 2; }
    // The transport test runs two independent listeners in the same process.
    kai::HttpServer secondHttp;
    kai::WebSocketStream secondStream(secondHttp.context(), R"({"type":"hello","version":1})");
    if (argc == 4 && !secondHttp.start("127.0.0.1", 0, argv[2], secondStream.upgradeHandler(), &error))
    { std::cerr << error << std::endl; return 2; }
    const int nPoints = argc == 4 ? std::stoi(argv[3]) : 10000;
    std::atomic<bool> running{true};
    std::thread producer([&] {
        std::vector<float> points;
        std::vector<uint8_t> colors;
        for (int i = 0; i < nPoints; ++i)
        {
            float a = i * 0.02f, z = (i % 100) / 50.f - 1;
            points.insert(points.end(), {std::cos(a), std::sin(a), z});
            colors.insert(colors.end(), {uint8_t(i % 256), 200, 240, 255});
        }
        float bounds[] = {-1,-1,-1,1,1,1};
        uint32_t sequence = 0;
        while (running)
        {
            auto frame = std::make_shared<std::vector<uint8_t>>();
            kai::webviewer3d::begin(*frame, ++sequence, 123456789);
            kai::webviewer3d::object(*frame, 7, 2, 1, bounds, points, colors,
                {-1,0,0,1,0,0}, {255,0,0,255,255,0,0,255});
            kai::webviewer3d::finish(*frame, 1);
            stream.publish(frame);
            if (argc == 4) secondStream.publish(frame);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });
    std::cout << http.port();
    if (argc == 4) std::cout << ' ' << secondHttp.port();
    std::cout << std::endl;
    std::string line;
    std::getline(std::cin, line);
    running = false;
    producer.join();
    http.stop();
    stream.stop();
    secondHttp.stop();
    secondStream.stop();
}
