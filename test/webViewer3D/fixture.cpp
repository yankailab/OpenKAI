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
    using namespace kai::webviewer3d;
    kai::HttpServer http, secondHttp;
    std::array<std::unique_ptr<kai::WebSocketStream>, 3> streams, secondStreams;
    std::vector<std::pair<std::string, kai::WebSocketStream *>> routes, secondRoutes;
    for (size_t i = 0; i < Types.size(); ++i)
    {
        const std::string hello = std::string(R"({"type":"hello","version":4,"stream":")") + name(Types[i]) +
            R"(","autoBound":true,"showGrid":true,"background":[0.035,0.045,0.065],"camera":{"eye":[0,-8,4],"target":[0,0,0],"up":[0,0,1],"type":0,"fov":70,"near":0.01,"far":1000000,"lr":[-10,10],"bt":[-10,10]},"objects":[{"id":7,"name":"Test cloud and lines"}]})";
        streams[i] = std::make_unique<kai::WebSocketStream>(http.context(), hello);
        secondStreams[i] = std::make_unique<kai::WebSocketStream>(secondHttp.context(), hello);
        routes.emplace_back(std::string("/stream/") + name(Types[i]), streams[i].get());
        secondRoutes.emplace_back(std::string("/stream/") + name(Types[i]), secondStreams[i].get());
    }
    std::string error;
    if (!http.start("127.0.0.1", 0, argv[1], kai::WebSocketStream::routes(routes), &error))
    { std::cerr << error << std::endl; return 2; }
    if (argc == 4 && !secondHttp.start("127.0.0.1", 0, argv[2], kai::WebSocketStream::routes(secondRoutes), &error))
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
		kai::OCTGRID_CELLS grid;
		grid.m_header = {{0, 0, 0}, {2, 2, 2}, 40, 123456789};
		kai::UUID128 path(0);
		for (unsigned depth = 0; depth <= 40; ++depth)
		{
			kai::OCTGRID_CELL cell;
			auto id = path;
			id |= uint64_t(depth);
			cell.setID(id);
			cell.m_vC[0] = 255; cell.m_vC[1] = 51;
			grid.m_vCell.push_back(cell);
			if (depth < 40) path |= kai::UUID128(7) << (123 - depth * 3);
		}
        uint32_t sequence = 0;
        while (running)
        {
            ++sequence;
            for (size_t i = 0; i < Types.size(); ++i)
            {
                auto frame = std::make_shared<std::vector<uint8_t>>();
                begin(*frame, Types[i], sequence, 123456789);
                if (Types[i] == Type::Points) kai::webviewer3d::points(*frame, 7, 2, 1, bounds, points, colors);
                else if (Types[i] == Type::Lines) lines(*frame, 7, 1, bounds, {-1,0,0,1,0,0}, {255,0,0,255,255,0,0,255});
                else cells(*frame, 7, 1, bounds, grid);
                finish(*frame, 1);
                streams[i]->publish(frame);
                if (argc == 4) secondStreams[i]->publish(frame);
            }
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
    for (auto &stream : streams) stream->stop();
    secondHttp.stop();
    for (auto &stream : secondStreams) stream->stop();
}
