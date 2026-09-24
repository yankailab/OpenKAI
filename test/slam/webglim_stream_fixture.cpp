#include "../../src/UI/Viewer/Web/WebGLIMProtocol.h"
#include <boost/asio/post.hpp>
#include "../../src/SLAM/_GLIM.h"
#include <future>
#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 2) return 1;
	using namespace kai;
	HttpServer http;
	const json hello = {{"type", "hello"}, {"protocol", "openkai.glim"}, {"version", webglim::Version},
		{"stream", "glim"}, {"maxChunkPoints", webglim::MaxChunkPoints}};
	webglim::Stream stream(http.context(), hello.dump());
	GLIM_MAP_SNAPSHOT snapshot;
	snapshot.session = 9007199254740993ULL; // Exercise JSON integers beyond JS Number precision.
	snapshot.revision = 1;
	stream.publish(snapshot);
	std::string error;
	if (!http.start("127.0.0.1", 0, argv[1], stream.upgradeHandler(), &error))
	{
		std::cerr << error << std::endl;
		return 2;
	}
	std::cout << json{{"port", http.port()}, {"session", std::to_string(snapshot.session)},
		{"maxChunkPoints", webglim::MaxChunkPoints}}.dump() << std::endl;
	for (std::string line; std::getline(std::cin, line);)
	{
		try
		{
			const auto request = json::parse(line);
			const string command = request.at("cmd");
			if (command == "quit") break;
			if (command == "append")
			{
				const size_t count = request.at("count");
				if (count > webglim::MaxSubmapPoints) throw std::runtime_error("fixture point limit");
				const size_t serial = snapshot.submaps.size() + 1;
				GLIM_SUBMAP submap;
				submap.id = 9007199254741092ULL + serial;
				submap.timestampUs = 9007199254750000ULL + serial;
				auto points = std::make_shared<vector<Vector3f>>();
				points->reserve(count);
				for (size_t i = 0; i < count; ++i) points->emplace_back(float(i), float(serial), -float(i % 97));
				submap.points = points;
				snapshot.submaps.push_back(submap);
				++snapshot.revision;
			}
			else if (command == "pose")
			{
				const uint64_t id = std::stoull(request.at("id").get<string>());
				bool found = false;
				for (auto &submap : snapshot.submaps) if (submap.id == id)
				{
					submap.pose.translation() = Vector3d(1, 2, 3);
					found = true;
				}
				if (!found) throw std::runtime_error("unknown fixture submap");
				++snapshot.revision;
			}
			else if (command == "reset")
			{
				++snapshot.session;
				++snapshot.revision;
				snapshot.submaps.clear();
			}
			else if (command != "same") throw std::runtime_error("unknown fixture command");
			stream.publish(snapshot);
			// Acknowledge only after the IO worker handles this publication.
			std::promise<void> barrier;
			auto done = barrier.get_future();
			boost::asio::post(http.context(), [&barrier] { barrier.set_value(); });
			done.get();
			json ids = json::array();
			for (const auto &submap : snapshot.submaps) ids.push_back(std::to_string(submap.id));
			std::cout << json{{"session", std::to_string(snapshot.session)}, {"revision", std::to_string(snapshot.revision)},
				{"ids", ids}}.dump() << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << json{{"error", e.what()}}.dump() << std::endl;
		}
	}
	http.stop();
	stream.stop();
	return 0;
}
