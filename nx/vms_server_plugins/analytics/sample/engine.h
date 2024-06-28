#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "detection.h"
#include <nlohmann/json.hpp>
#include <nx/sdk/analytics/helpers/engine.h>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>
#include <nx/sdk/i_device_info.h>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

class Engine: public nx::sdk::analytics::Engine
{
public:
    Engine(const std::filesystem::path& pluginHomeDir);
    ~Engine();

    void processColorDetection(const std::string& message);
    void doObtainDeviceAgent(nx::sdk::Result<nx::sdk::analytics::IDeviceAgent*>* outResult, const nx::sdk::IDeviceInfo* deviceInfo);
    std::string manifestString() const;

private:
    std::filesystem::path m_pluginHomeDir;
    std::vector<Detection> m_detections;
};

} // namespace opencv_object_detection
} // namespace vms_server_plugins
} // namespace sample_company
} // namespace nx