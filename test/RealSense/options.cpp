#include "../../src/Vision/RGBD/RealSenseOptions.h"
#include <librealsense2/hpp/rs_internal.hpp>
#include <iostream>
#include <set>

using namespace kai::realsense;

namespace
{
    void require(bool result, const std::string &message)
    {
        if (!result) throw std::runtime_error(message);
    }

    template<class F> void rejects(F operation, const std::string &message)
    {
        try { operation(); }
        catch (const std::invalid_argument &) { return; }
        throw std::runtime_error("Expected validation failure: " + message);
    }

    void testOptionNames()
    {
        std::set<std::string> keys;
        for (int i = 0; i < RS2_OPTION_COUNT; ++i)
        {
            const auto id = static_cast<rs2_option>(i);
            const auto name = optionName(id);
            require(!name.empty(), "Missing canonical option name");
            require(optionId(name) == id, "Canonical option name does not round trip");
            require(keys.insert(name).second, "Duplicate canonical option name");
        }
        require(optionName(RS2_OPTION_DIGITAL_GAIN) == "RS2_OPTION_DIGITAL_GAIN", "Digital gain is canonical");
        require(optionId("typo") == RS2_OPTION_COUNT, "Unknown key rejected");
        require(optionName(static_cast<rs2_option>(-1)).empty(), "Invalid negative ID rejected");
        require(optionName(RS2_OPTION_COUNT).empty(), "Sentinel ID rejected");
    }

    void testSoftwareOptions()
    {
        // Software-only device: constructing this does not enumerate or open USB.
        rs2::software_device device;
        auto sensor = device.add_sensor("OpenKAI option validation test");
        sensor.add_option(RS2_OPTION_EXPOSURE, {0, 100, 10, 2}); // min, max, default, step
        sensor.add_option(RS2_OPTION_GAIN, {0.1f, 0.9f, 0.1f, 0.1f});
        sensor.add_read_only_option(RS2_OPTION_ASIC_TEMPERATURE, 35);
        sensor.add_option(RS2_OPTION_AUTO_GAIN_LIMIT, {16, 248, 16, 1}, false);
        require(optionValue(sensor, RS2_OPTION_EXPOSURE) == 10, "Read SDK default");
        setOption(sensor, RS2_OPTION_EXPOSURE, 12);
        require(optionValue(sensor, RS2_OPTION_EXPOSURE) == 12, "Write/read option");
        setOption(sensor, RS2_OPTION_GAIN, 0.1);
        require(optionValue(sensor, RS2_OPTION_GAIN).get<float>() == 0.1f, "Decimal boundary survives float conversion");

        rejects([&] { setOption(sensor, RS2_OPTION_EXPOSURE, 13); }, "nonmultiple of step");
        rejects([&] { setOption(sensor, RS2_OPTION_EXPOSURE, -2); }, "below minimum");
        rejects([&] { setOption(sensor, RS2_OPTION_EXPOSURE, 102); }, "above maximum");
        rejects([&] { setOption(sensor, RS2_OPTION_EXPOSURE, "12"); }, "numeric string");
        rejects([&] { setOption(sensor, RS2_OPTION_EXPOSURE, true); }, "boolean for float option");
        rejects([&] { setOption(sensor, RS2_OPTION_EXPOSURE, nullptr); }, "caller handles null");
        rejects([&] { setOption(sensor, RS2_OPTION_EXPOSURE, std::numeric_limits<double>::infinity()); }, "infinity");
        rejects([&] { setOption(sensor, RS2_OPTION_EXPOSURE, std::numeric_limits<double>::quiet_NaN()); }, "NaN");
        rejects([&] { setOption(sensor, RS2_OPTION_ASIC_TEMPERATURE, 30); }, "read-only telemetry");
        rejects([&] { setOption(sensor, RS2_OPTION_BRIGHTNESS, 0); }, "unsupported option");
        validateOption(sensor, RS2_OPTION_AUTO_GAIN_LIMIT, 64, false);
        rejects([&] { validateOption(sensor, RS2_OPTION_AUTO_GAIN_LIMIT, 64); }, "default validation checks writability");
        rejects([&] { validateOption(sensor, RS2_OPTION_AUTO_GAIN_LIMIT, 500, false); }, "writability bypass still checks range");
        rejects([&] { setOption(sensor, RS2_OPTION_AUTO_GAIN_LIMIT, 64); }, "writes always check writability");
        require(optionValue(sensor, RS2_OPTION_EXPOSURE) == 12, "Rejected writes leave the value intact");
        require(optionValue(sensor, RS2_OPTION_ASIC_TEMPERATURE) == 35, "Read-only telemetry remains intact");

        const json schema = optionSchema(sensor, RS2_OPTION_EXPOSURE);
        require(schema["key"] == "RS2_OPTION_EXPOSURE", "Schema uses canonical key");
        require(schema["type"] == "float", "Schema preserves SDK type");
        require(schema["min"] == 0 && schema["max"] == 100 && schema["step"] == 2, "Schema range");
        require(schema["default"] == 10 && schema["current"] == 12, "Schema default and current are distinct");
        require(schema["nullable"] == true && schema["supported"] == true && schema["readOnly"] == false, "Schema capabilities");
        require(optionSchema(sensor, RS2_OPTION_ASIC_TEMPERATURE)["readOnly"] == true, "Schema marks telemetry read-only");
    }

    void testFilterOptions()
    {
        rs2::decimation_filter filter;
        setOption(filter, RS2_OPTION_FILTER_MAGNITUDE, 3);
        require(optionValue(filter, RS2_OPTION_FILTER_MAGNITUDE) == 3, "Processing-block option write/read");
        rejects([&] { setOption(filter, RS2_OPTION_FILTER_MAGNITUDE, 2.5); }, "filter step");
        rs2::hole_filling_filter holeFilling;
        const auto schema = optionSchema(holeFilling, RS2_OPTION_HOLES_FILL);
        require(schema.contains("choices") && schema["choices"].size() == 3, "Filter enum descriptions exposed");
    }

    void testTypedValues()
    {
        require(optionValueJSON(rs2::option_value(RS2_OPTION_SEQUENCE_NAME, "name")) == "name", "Packed SDK string conversion");
        require(optionValueJSON(rs2::option_value(RS2_OPTION_SEQUENCE_ID, int64_t(17))) == 17, "Packed SDK integer conversion");
        require(optionValueJSON(rs2::option_value(RS2_OPTION_HDR_ENABLED, true)) == true, "SDK boolean conversion");
        require(optionValueJSON(rs2::option_value(RS2_OPTION_EXPOSURE, 2.5f)) == 2.5, "SDK float conversion");
        require(optionValueJSON(rs2::option_value(RS2_OPTION_EXPOSURE, rs2::option_value::invalid)).is_null(), "Unavailable value stays null");
        require(optionValueJSON(rs2::option_value(RS2_OPTION_EXPOSURE, std::numeric_limits<float>::infinity())).is_null(), "Nonfinite telemetry stays null");
        rs2::option_value rectangle(RS2_OPTION_REGION_OF_INTEREST, false);
        auto *raw = const_cast<rs2_option_value *>(static_cast<const rs2_option_value *>(rectangle));
        raw->type = RS2_OPTION_TYPE_RECT;
        raw->as_rect = {1, 2, 3, 4};
        require(optionValueJSON(rectangle) == json::array({1, 2, 3, 4}), "SDK rectangle conversion");
        require(optionInteger(json(INT64_MAX)) == INT64_MAX, "Maximum signed integer is preserved");
        rejects([] { optionInteger(json(UINT64_MAX)); }, "unsigned integer overflow");
        rejects([] { optionInteger(json(1.5)); }, "fractional integer");
    }
}

int main()
{
    try
    {
        testOptionNames();
        testSoftwareOptions();
        testFilterOptions();
        testTypedValues();
        std::cout << "RealSense options: canonical names, validation, schema, filters and typed values passed\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
