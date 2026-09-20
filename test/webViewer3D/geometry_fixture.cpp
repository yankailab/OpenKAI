#include "../../src/UI/Viewer/Web/_WebGeometryBase.h"
#include "../../src/Universe/Geometry/PointCloud/_PointCloud.h"
#include "../../src/Universe/Geometry/Line/_Line.h"
#include "../../src/Module/ModuleMgr.h"
#include <cassert>
#include <iostream>

using namespace kai;

class Sources : public ModuleMgr
{
public:
    void add(BASE *source) { m_vModules.push_back(source); }
    ~Sources() { m_vModules.clear(); }
};

int main(int argc, char **argv)
{
    assert(argc == 5);
    const json thread = {{"FPS", 30}};
    Sources sources;
    _PointCloud points;
    _Line lines;
    _GeometryBase empty;
    points.setName("points"); lines.setName("lines"); empty.setName("empty");
    assert(points.init(json{{"class", "_PointCloud"}, {"thread", thread}, {"nP", 16}}));
    assert(lines.init(json{{"class", "_Line"}, {"thread", thread}, {"nL", 16}}));
    sources.add(&points); sources.add(&lines); sources.add(&empty);
    // Two completed frames, then an unfinished frame with a different timestamp.
    points.frameStart();
    points.add({-1, 0, 0}, {1, 0, 0}, 10);
    points.frameStop();
    points.frameStart();
    points.add({0, 0, 0}, {0, 1, 0}, 20);
    points.add({1, 0, 0}, {0, 0, 1}, 20);
    points.frameStop();
    points.frameStart();
    points.add({2, 0, 0}, {1, 1, 0}, 30);
    lines.add({-1, 0, 0}, {1, 0, 0}, {1, 0, 1}, 20);

    vector<Vector3f> positions, colors;
    uint64_t timestamp = 0;
    _GeometryBase *base = &points;
    assert(base->getLastFrame(&positions, &colors, timestamp) == 2);
    assert(timestamp == 20 && positions[0].x() == 0 && positions[1].x() == 1);
    assert(colors[0].y() == 1 && colors[1].z() == 1);
    assert(empty.getLastFrame(&positions, &colors, timestamp) == 0);
    assert(positions.empty() && colors.empty() && timestamp == 0);

    _WebGeometryBase viewer;
    viewer.setName("viewer");
    json config = {{"class", "_WebGeometryBase"}, {"thread", thread},
        {"host", "127.0.0.1"}, {"port", std::stoi(argv[2])}, {"webRoot", argv[1]},
        {"bFrame", std::stoi(argv[3]) != 0}, {"dTexpire", std::stoi(argv[4])},
        {"nPbuf", 16}, {"nLbuf", 16},
        {"vGeometry", {{{"_GeometryBase", "points"}, {"nP", 16}, {"nL", 0}},
                       {{"_GeometryBase", "lines"}, {"nP", 0}, {"nL", 16}},
                       {{"_GeometryBase", "empty"}, {"nP", 16}, {"nL", 0}}}}};
    assert(viewer.init(config));
    for (const json &entry : {json{{"_GeometryBase", "missing"}},
                             json{{"_GeometryBase", "points"}, {"nP", -1}}})
    {
        auto bad = config; bad["vGeometry"] = {entry};
        assert(!viewer.link(bad, &sources));
    }
    auto bad = config; bad["vSelectableOctGrid"] = json::array();
    assert(!viewer.link(bad, &sources));
    assert(viewer.link(config, &sources));
    assert(viewer.start());
    std::cout << "READY" << std::endl;
    std::string command;
    while (std::getline(std::cin, command) && command != "stop")
    {
        if (command == "clear") { points.clear(); lines.clear(); }
        std::cout << "OK" << std::endl;
    }
    viewer.stop();
}
