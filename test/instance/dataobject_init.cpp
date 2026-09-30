#include "Base/_ModuleBase.h"
#include "DataObject/BBoxStream.h"
#include "Instance/InstanceMgr.h"
#include <iostream>
#include <stdexcept>

using namespace kai;

static void require(bool condition, const string &message)
{
    if (!condition)
        throw std::runtime_error(message);
}

static void testDetectorConfig(const string &filename)
{
    JsonCfg config;
    require(config.readFromFile(filename), "Could not read Detectors.json");
    InstanceMgr manager;
    require(manager.loadJsonFiles(filename), "Could not load Detectors.json");

    // Keep the real detection stream configuration; skip camera, GUI, and inference.
    for (auto it = config.getJson()->begin(); it != config.getJson()->end(); ++it)
    {
        if (it.key() != "detections" && it.value().is_object())
            (*manager.findJson(it.key()))["bON"] = false;
    }
    require(manager.findJson("detections")->at("nBuf") == 1,
            "This regression expects Detectors.json detections.nBuf=1");
    require(manager.createAll(), "Could not create configured objects");
    require(manager.initAll(), "Could not initialize configured objects");
    auto *stream = static_cast<BBoxStream *>(manager.findDataObject("detections"));
    require(stream != nullptr, "Missing configured BBoxStream");

    vector<BBOX_OBJ> result;
    for (uint64_t frame = 1; frame <= 4; ++frame)
    {
        vector<BBOX_OBJ> batch(3);
        for (size_t i = 0; i < batch.size(); ++i)
        {
            batch[i].setType(obj_bbox);
            batch[i].setPos(Vector3f(frame * 10 + i, 20, 0));
            batch[i].setDim(Vector3f(10, 10, 0));
            batch[i].m_tStamp = frame;
        }
        stream->add(batch, frame);
        require(stream->get(result) == frame, "Incorrect stream timestamp");
        require(result.size() == 1,
                "nBuf=1 retained " + std::to_string(result.size()) + " boxes");
        require(result.front().m_vPos == batch.back().m_vPos,
                "nBuf=1 did not retain the newest detection");
    }
    require(stream->getClass() == "BBoxStream", "DataObject base configuration was not loaded");
}

class ProbeDataObject : public DataObjBase
{
public:
    ProbeDataObject(vector<string> &events, bool succeeds)
        : m_events(events), m_succeeds(succeeds) {}
    bool loadConfig() override
    {
        m_events.push_back("dataObject");
        return m_succeeds;
    }
private:
    vector<string> &m_events;
    bool m_succeeds;
};

class ProbeModule : public _ModuleBase
{
public:
    explicit ProbeModule(vector<string> &events) : m_events(events) {}
    bool loadConfig() override
    {
        m_events.push_back("module");
        return true;
    }
private:
    vector<string> &m_events;
};

class ProbeManager : public InstanceMgr
{
public:
    void addProbe(DataObjBase *object) { m_vDataStreams.push_back(object); }
    void addProbe(_ModuleBase *module) { m_vModules.push_back(module); }
};

static void testInitializationOrder(bool dataObjectSucceeds)
{
    vector<string> events;
    ProbeManager manager;
    manager.addProbe(new ProbeDataObject(events, dataObjectSucceeds));
    manager.addProbe(new ProbeModule(events));
    require(manager.initAll() == dataObjectSucceeds, "DataObject initialization failure was ignored");
    const vector<string> expected = dataObjectSucceeds
        ? vector<string>{"dataObject", "module"} : vector<string>{"dataObject"};
    require(events == expected, "Modules must initialize only after successful DataObject initialization");
}

int main(int argc, char **argv)
{
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;
    FLAGS_minloglevel = 2;
    try
    {
        require(argc == 2, "Expected path to Detectors.json");
        testDetectorConfig(argv[1]);
        testInitializationOrder(true);
        testInitializationOrder(false);
        std::cout << "PASS: configured nBuf=1, initialization order, and failure propagation\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
