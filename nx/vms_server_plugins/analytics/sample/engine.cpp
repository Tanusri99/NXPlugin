#include "engine.h"
#include "device_agent.h"
#include "kafka_consumer.h"
#include <nlohmann/json.hpp> // For JSON.
#include <curl/curl.h> // For HTTP requests.
#include <iostream> // For std::cout.
#include <filesystem> // For std::filesystem::path.
#include <nx/sdk/analytics/helpers/plugin.h>
#include <nx/sdk/analytics/helpers/engine.h>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Engine::Engine(const std::filesystem::path& pluginHomeDir):
    nx::sdk::analytics::Engine(/*enableOutput*/ true),
    m_pluginHomeDir(pluginHomeDir)
{
}

Engine::~Engine()
{
    //Cleanup code
}

void Engine::doObtainDeviceAgent(nx::sdk::Result<nx::sdk::analytics::IDeviceAgent*>* outResult, const nx::sdk::IDeviceInfo* deviceInfo)
{
    *outResult = new DeviceAgent(deviceInfo, m_pluginHomeDir);
}

void Engine::processColorDetection(const std::string& colorMessage)
{
    nlohmann::json message = nlohmann::json::parse(colorMessage);
    Detection detection;
    std::string label = message["label"];
    float confidence = message["confidence"];
    std::string topColors = message["top_colors"];
    std::string pantsColors = message["pants_colors"];
    std::string dominantColors = message["dominant_colors"];

    if (label == "person") {
        std::cout << "Detected person with top color: " << topColors << " and pants color: " << pantsColors << std::endl;
    } else {
        std::cout << "Detected " << label << " with dominant color: " << dominantColors << std::endl;
    }
    m_detections.push_back(detection);
}

std::string Engine::manifestString() const
{
    return /*suppress newline*/ 1 + R"json(
    {
        "id": "sample.opencv_object_detection",
        "name": "OpenCV object detection",
        "description": "This plugin is for object detection and color detection using OpenCV.",
        "version": "1.1.0",
        "vendor": "Sample Inc."
    }
    )json";
}

} // namespace opencv_object_detection
} // namespace vms_server_plugins
} // namespace sample_company
} // namespace nx