// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/ptr.h>

#include "plugin.h"
#include "engine.h"
#include "device_agent.h"
#include "plugin_settings.h"
#include "applogger.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>
// Add this line to include the missing header
#include "kafka_consumer.h"
#include <iostream>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Result<IEngine*> Plugin::doObtainEngine()
{
    // const auto utilityProvider = this->utilityProvider();
    // const std::filesystem::path pluginHomeDir = utilityProvider->homeDir();
    // auto engine = new Engine(pluginHomeDir);

    // //Create a temporary DeviceAgent to fetch the nxCamId
    // std::string nxCamId = "";
    // auto deviceAgent = new DeviceAgent(nullptr, pluginHomeDir);
    // nxCamId = deviceAgent->getNxCamId();
    // delete deviceAgent;

    // // Start instance segmentation via Flask app
    // CURL* curl;
    // CURLcode res;

    // curl = curl_easy_init();
    // if(curl) {
    //     struct curl_slist *headers = NULL;
    //     headers = curl_slist_append(headers, "Content-Type: application/json");
    //     std::string jsonPayload = "{\"nx_cam_id\": \"" + nxCamId + "\"}";
    //     curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/start_detection");
    //     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        
    //     res = curl_easy_perform(curl);
    //     if(res != CURLE_OK)
    //         fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        
    //     curl_easy_cleanup(curl);
    // }

    // // Initialize Kafka consumer and set up the detection callbacks
    // KafkaConsumer consumer("localhost:9092", "color_detection");
    // consumer.setMessageCallback([engine](const std::string& message) {
    //     engine->processColorDetection(message);
    // });
    // consumer.start();

    return new engine;
}

std::string Plugin::manifestString() const
{
    return /*suppress newline*/ 1 + R"json(
{
    "id": "aol.colordetection.plugin",
    "name": "AOL Color Detection",
    "description": "This plugin is for object detection and color detection. It's based on OpenCV.",
    "version": "1.0.0",
    "vendor": "Art of Logic",
    "engineSettingsModel": {
        
        "type": "Settings",
		"items": [
            {
                "type": "GroupBox",
			    "caption": "Application settings",
			    "items": [
                    {
						"type": "CheckBox",
						"name": ")json" + kThrowPluginDiagnostics + R"json(",
						"caption": "Produce Plugin Diagnostic Events?",
						"defaultValue": true
					},
					{
                        "type": "TextField",
                        "name": ")json" + kKafkaIP + R"json(",
                        "caption": "Kafka IP",
                        "defaultValue": "127.0.0.1"
                    },
					{
                        "type": "TextField",
                        "name": ")json" + kKafkaPort + R"json(",
                        "caption": "Kafka Port",
                        "defaultValue": "9093"
                    }
                ]
            }
        }
    }
)json";
}

extern "C" NX_PLUGIN_API nx::sdk::IPlugin* createNxPlugin()
{
    AppLogger::debug("Creating plugin");
    return new Plugin();
}

} // namespace opencv_object_detection
} // namespace vms_server_plugins
} // namespace sample_company
}