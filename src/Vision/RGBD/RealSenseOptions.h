// RealSense SDK 2.58 typed options shared by configuration and the web controls.
#ifndef OPENKAI_REALSENSE_OPTIONS_H
#define OPENKAI_REALSENSE_OPTIONS_H

#include <librealsense2/rs.hpp>
#include "../../Dependencies/json.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

namespace kai::realsense
{
    using json = nlohmann::json;

    // Indexed by SDK ID, with exactly one canonical key per supported option.
    inline constexpr const char *optionNames[] = {
        "RS2_OPTION_BACKLIGHT_COMPENSATION",
        "RS2_OPTION_BRIGHTNESS",
        "RS2_OPTION_CONTRAST",
        "RS2_OPTION_EXPOSURE",
        "RS2_OPTION_GAIN",
        "RS2_OPTION_GAMMA",
        "RS2_OPTION_HUE",
        "RS2_OPTION_SATURATION",
        "RS2_OPTION_SHARPNESS",
        "RS2_OPTION_WHITE_BALANCE",
        "RS2_OPTION_ENABLE_AUTO_EXPOSURE",
        "RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE",
        "RS2_OPTION_VISUAL_PRESET",
        "RS2_OPTION_LASER_POWER",
        "RS2_OPTION_ACCURACY",
        "RS2_OPTION_MOTION_RANGE",
        "RS2_OPTION_FILTER_OPTION",
        "RS2_OPTION_CONFIDENCE_THRESHOLD",
        "RS2_OPTION_EMITTER_ENABLED",
        "RS2_OPTION_FRAMES_QUEUE_SIZE",
        "RS2_OPTION_TOTAL_FRAME_DROPS",
        "RS2_OPTION_AUTO_EXPOSURE_MODE",
        "RS2_OPTION_POWER_LINE_FREQUENCY",
        "RS2_OPTION_ASIC_TEMPERATURE",
        "RS2_OPTION_ERROR_POLLING_ENABLED",
        "RS2_OPTION_PROJECTOR_TEMPERATURE",
        "RS2_OPTION_OUTPUT_TRIGGER_ENABLED",
        "RS2_OPTION_MOTION_MODULE_TEMPERATURE",
        "RS2_OPTION_DEPTH_UNITS",
        "RS2_OPTION_ENABLE_MOTION_CORRECTION",
        "RS2_OPTION_AUTO_EXPOSURE_PRIORITY",
        "RS2_OPTION_COLOR_SCHEME",
        "RS2_OPTION_HISTOGRAM_EQUALIZATION_ENABLED",
        "RS2_OPTION_MIN_DISTANCE",
        "RS2_OPTION_MAX_DISTANCE",
        "RS2_OPTION_TEXTURE_SOURCE",
        "RS2_OPTION_FILTER_MAGNITUDE",
        "RS2_OPTION_FILTER_SMOOTH_ALPHA",
        "RS2_OPTION_FILTER_SMOOTH_DELTA",
        "RS2_OPTION_HOLES_FILL",
        "RS2_OPTION_STEREO_BASELINE",
        "RS2_OPTION_AUTO_EXPOSURE_CONVERGE_STEP",
        "RS2_OPTION_INTER_CAM_SYNC_MODE",
        "RS2_OPTION_STREAM_FILTER",
        "RS2_OPTION_STREAM_FORMAT_FILTER",
        "RS2_OPTION_STREAM_INDEX_FILTER",
        "RS2_OPTION_EMITTER_ON_OFF",
        "RS2_OPTION_ZERO_ORDER_POINT_X",
        "RS2_OPTION_ZERO_ORDER_POINT_Y",
        "RS2_OPTION_LLD_TEMPERATURE",
        "RS2_OPTION_MC_TEMPERATURE",
        "RS2_OPTION_MA_TEMPERATURE",
        "RS2_OPTION_HARDWARE_PRESET",
        "RS2_OPTION_GLOBAL_TIME_ENABLED",
        "RS2_OPTION_APD_TEMPERATURE",
        "RS2_OPTION_ENABLE_MAPPING",
        "RS2_OPTION_ENABLE_RELOCALIZATION",
        "RS2_OPTION_ENABLE_POSE_JUMPING",
        "RS2_OPTION_ENABLE_DYNAMIC_CALIBRATION",
        "RS2_OPTION_DEPTH_OFFSET",
        "RS2_OPTION_LED_POWER",
        "RS2_OPTION_ZERO_ORDER_ENABLED",
        "RS2_OPTION_ENABLE_MAP_PRESERVATION",
        "RS2_OPTION_FREEFALL_DETECTION_ENABLED",
        "RS2_OPTION_AVALANCHE_PHOTO_DIODE",
        "RS2_OPTION_POST_PROCESSING_SHARPENING",
        "RS2_OPTION_PRE_PROCESSING_SHARPENING",
        "RS2_OPTION_NOISE_FILTERING",
        "RS2_OPTION_INVALIDATION_BYPASS",
        "RS2_OPTION_DIGITAL_GAIN",
        "RS2_OPTION_SENSOR_MODE",
        "RS2_OPTION_EMITTER_ALWAYS_ON",
        "RS2_OPTION_THERMAL_COMPENSATION",
        "RS2_OPTION_TRIGGER_CAMERA_ACCURACY_HEALTH",
        "RS2_OPTION_RESET_CAMERA_ACCURACY_HEALTH",
        "RS2_OPTION_HOST_PERFORMANCE",
        "RS2_OPTION_HDR_ENABLED",
        "RS2_OPTION_SEQUENCE_NAME",
        "RS2_OPTION_SEQUENCE_SIZE",
        "RS2_OPTION_SEQUENCE_ID",
        "RS2_OPTION_HUMIDITY_TEMPERATURE",
        "RS2_OPTION_ENABLE_MAX_USABLE_RANGE",
        "RS2_OPTION_ALTERNATE_IR",
        "RS2_OPTION_NOISE_ESTIMATION",
        "RS2_OPTION_ENABLE_IR_REFLECTIVITY",
        "RS2_OPTION_AUTO_EXPOSURE_LIMIT",
        "RS2_OPTION_AUTO_GAIN_LIMIT",
        "RS2_OPTION_AUTO_RX_SENSITIVITY",
        "RS2_OPTION_TRANSMITTER_FREQUENCY",
        "RS2_OPTION_VERTICAL_BINNING",
        "RS2_OPTION_RECEIVER_SENSITIVITY",
        "RS2_OPTION_AUTO_EXPOSURE_LIMIT_TOGGLE",
        "RS2_OPTION_AUTO_GAIN_LIMIT_TOGGLE",
        "RS2_OPTION_EMITTER_FREQUENCY",
        "RS2_OPTION_DEPTH_AUTO_EXPOSURE_MODE",
        "RS2_OPTION_OHM_TEMPERATURE",
        "RS2_OPTION_SOC_PVT_TEMPERATURE",
        "RS2_OPTION_GYRO_SENSITIVITY",
        "RS2_OPTION_REGION_OF_INTEREST",
        "RS2_OPTION_ROTATION",
        "RS2_OPTION_SAFETY_PRESET_ACTIVE_INDEX",
        "RS2_OPTION_SAFETY_MODE",
        "RS2_OPTION_RGB_TNR_ENABLED",
        "RS2_OPTION_SAFETY_MCU_TEMPERATURE",
        "RS2_OPTION_LEFT_IR_TEMPERATURE",
        "RS2_OPTION_EMBEDDED_FILTER_ENABLED",
        "RS2_OPTION_DISPARITY_SHIFT",
        "RS2_OPTION_THRESHOLD",
        "RS2_OPTION_DOWNSCALE_RATIO",
        "RS2_OPTION_READOUT_SHAPING",
        "RS2_OPTION_DETECTION_DISTANCE",
        "RS2_OPTION_SENSORS_CONFIG_MODE",

    };
    static_assert(sizeof(optionNames) / sizeof(optionNames[0]) == RS2_OPTION_COUNT,
                  "Update RealSense option names for this SDK version");

    inline std::string optionName(rs2_option id)
    {
        const auto i = static_cast<int>(id);
        return i >= 0 && i < RS2_OPTION_COUNT ? optionNames[i] : "";
    }

    inline rs2_option optionId(const std::string &name)
    {
        for (int i = 0; i < RS2_OPTION_COUNT; ++i)
            if (name == optionNames[i]) return static_cast<rs2_option>(i);
        return RS2_OPTION_COUNT;
    }

    inline void requireOption(rs2::options &options, rs2_option id)
    {
        if (optionName(id).empty() || !options.supports(id))
            throw std::invalid_argument("Unsupported RealSense option: " + optionName(id));
    }

    inline json optionValueJSON(const rs2::option_value &value)
    {
        if (!value->is_valid) return nullptr;
        switch (value->type)
        {
        case RS2_OPTION_TYPE_FLOAT:
            return std::isfinite(value->as_float) ? json(value->as_float) : json(nullptr);
        case RS2_OPTION_TYPE_INTEGER: return int64_t(value->as_integer);
        case RS2_OPTION_TYPE_BOOLEAN: return value->as_integer != 0;
        case RS2_OPTION_TYPE_STRING:
        {
            // Copy out of the SDK packed union before JSON binds a reference.
            const char *text = value->as_string;
            return text ? json(text) : json(nullptr);
        }
        case RS2_OPTION_TYPE_RECT:
            return json::array({value->as_rect.x1, value->as_rect.y1,
                                value->as_rect.x2, value->as_rect.y2});
        default: throw std::runtime_error("Unknown RealSense option value type");
        }
    }

    inline json optionValue(rs2::options &options, rs2_option id)
    {
        requireOption(options, id);
        return optionValueJSON(options.get_option_value(id));
    }

    inline const char *optionTypeName(rs2_option_type type)
    {
        switch (type)
        {
        case RS2_OPTION_TYPE_FLOAT: return "float";
        case RS2_OPTION_TYPE_INTEGER: return "int";
        case RS2_OPTION_TYPE_BOOLEAN: return "bool";
        case RS2_OPTION_TYPE_STRING: return "string";
        case RS2_OPTION_TYPE_RECT: return "rect";
        default: throw std::runtime_error("Unknown RealSense option value type");
        }
    }

    inline int64_t optionInteger(const json &value)
    {
        if (!value.is_number_integer() ||
            (value.is_number_unsigned() && value.get<uint64_t>() > uint64_t(INT64_MAX)))
            throw std::invalid_argument("Expected a signed 64-bit integer");
        return value.get<int64_t>();
    }

    inline void validateOption(rs2::options &options, rs2_option id, const json &value, bool checkWritable = true)
    {
        requireOption(options, id);
        if (checkWritable && options.is_option_read_only(id))
            throw std::invalid_argument(optionName(id) + " is read-only");
        const auto type = options.get_option_value(id)->type;
        double number = 0;
        switch (type)
        {
        case RS2_OPTION_TYPE_FLOAT:
            if (!value.is_number()) throw std::invalid_argument("Expected a number");
            number = value.get<double>();
            if (!std::isfinite(number) || std::abs(number) > std::numeric_limits<float>::max())
                throw std::invalid_argument("Expected a finite float");
            // Compare the SDK float representation, so a decimal boundary such
            // as 0.1 is accepted when the range contains its float equivalent.
            number = static_cast<float>(number);
            break;
        case RS2_OPTION_TYPE_INTEGER:
            number = static_cast<double>(optionInteger(value));
            break;
        case RS2_OPTION_TYPE_BOOLEAN:
            if (!value.is_boolean()) throw std::invalid_argument("Expected a boolean");
            number = value.get<bool>() ? 1 : 0;
            break;
        case RS2_OPTION_TYPE_STRING:
            if (!value.is_string()) throw std::invalid_argument("Expected a string");
            if (value.get_ref<const std::string &>().find('\0') != std::string::npos)
                throw std::invalid_argument("Option strings cannot contain NUL bytes");
            return;
        case RS2_OPTION_TYPE_RECT:
        {
            if (!value.is_array() || value.size() != 4)
                throw std::invalid_argument("Expected rectangle [x1,y1,x2,y2]");
            for (const auto &coordinate : value)
            {
                const auto n = optionInteger(coordinate);
                if (n < 0 || n > INT16_MAX)
                    throw std::invalid_argument("Rectangle coordinates must be in [0,32767]");
            }
            if (value[0].get<int>() > value[2].get<int>() || value[1].get<int>() > value[3].get<int>())
                throw std::invalid_argument("Rectangle minimum must not exceed maximum");
            return;
        }
        default: throw std::runtime_error("Unknown RealSense option value type");
        }

        const auto range = options.get_option_range(id);
        if (number < range.min || number > range.max)
            throw std::invalid_argument(optionName(id) + " is outside the device range");
        if (std::isfinite(range.step) && range.step > 0)
        {
            const double steps = (number - range.min) / range.step;
            const double tolerance = std::max(1e-4, std::abs(steps) * 4 * std::numeric_limits<float>::epsilon());
            if (std::abs(steps - std::round(steps)) > tolerance)
                throw std::invalid_argument(optionName(id) + " does not match the device step");
        }
    }

    inline void setOption(rs2::options &options, rs2_option id, const json &value)
    {
        validateOption(options, id, value);
        const auto type = options.get_option_value(id)->type;
        // The SDK's public constructors allocate a mutable value and own it with
        // shared_ptr. Fill that value to support RECT too (no RECT constructor).
        // Do not pass a locally allocated struct to the SDK-handle constructor:
        // its deleter is exclusively for values allocated by the C API.
        rs2::option_value typed(id, false);
        auto *raw = const_cast<rs2_option_value *>(static_cast<const rs2_option_value *>(typed));
        raw->type = type;
        const std::string stringValue = value.is_string() ? value.get<std::string>() : "";
        switch (type)
        {
        case RS2_OPTION_TYPE_FLOAT: raw->as_float = value.get<float>(); break;
        case RS2_OPTION_TYPE_INTEGER: raw->as_integer = optionInteger(value); break;
        case RS2_OPTION_TYPE_BOOLEAN: raw->as_integer = value.get<bool>() ? 1 : 0; break;
        case RS2_OPTION_TYPE_STRING: raw->as_string = stringValue.c_str(); break;
        case RS2_OPTION_TYPE_RECT:
            raw->as_rect = {value[0].get<int16_t>(), value[1].get<int16_t>(),
                            value[2].get<int16_t>(), value[3].get<int16_t>()};
            break;
        default: throw std::runtime_error("Unknown RealSense option value type");
        }
        options.set_option_value(typed);
    }

    inline json optionSchema(rs2::options &options, rs2_option id)
    {
        requireOption(options, id);
        const auto value = options.get_option_value(id);
        const char *description = options.get_option_description(id);
        json schema = {{"key", optionName(id)}, {"type", optionTypeName(value->type)},
                       {"nullable", true}, {"supported", true},
                       {"readOnly", options.is_option_read_only(id)},
                       {"current", optionValueJSON(value)},
                       {"description", description ? description : ""}};
        const auto range = options.get_option_range(id);
        if (value->type != RS2_OPTION_TYPE_RECT && value->type != RS2_OPTION_TYPE_STRING)
        {
            if (std::isfinite(range.min)) schema["min"] = range.min;
            if (std::isfinite(range.max)) schema["max"] = range.max;
            if (std::isfinite(range.step)) schema["step"] = range.step;
            if (std::isfinite(range.def)) schema["default"] = range.def;
        }

        // Enum descriptions are optional; only emit a select when every value
        // has a label. Bound the work for numeric ranges such as exposure.
        if (std::isfinite(range.min) && std::isfinite(range.max) &&
            std::isfinite(range.step) && range.step > 0 && range.max >= range.min)
        {
            const double steps = (double(range.max) - range.min) / range.step;
            if (steps <= 256 && std::abs(steps - std::round(steps)) < 1e-4)
            {
                json choices = json::array();
                for (int i = 0; i <= static_cast<int>(std::round(steps)); ++i)
                {
                    const float number = range.min + i * range.step;
                    const char *label = nullptr;
                    try { label = options.get_option_value_description(id, number); }
                    catch (const rs2::error &) { break; }
                    if (!label || !*label) break;
                    json choice = number;
                    if (value->type == RS2_OPTION_TYPE_STRING) choice = label;
                    else if (value->type == RS2_OPTION_TYPE_BOOLEAN) choice = number != 0;
                    else if (value->type == RS2_OPTION_TYPE_INTEGER) choice = int64_t(number);
                    choices.push_back({{"value", choice}, {"label", label}});
                }
                if (!choices.empty() && choices.size() == size_t(std::round(steps)) + 1)
                    schema["choices"] = std::move(choices);
            }
        }
        return schema;
    }
}
#endif
