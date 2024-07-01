#include "engine.h"
#include "device_agent.h"
#include "kafka_consumer.h"
#include "plugin_settings.h"
#include "applogger.h"
#include <sstream>
#include <nlohmann/json.hpp> // For JSON.
#include <curl/curl.h> // For HTTP requests.
#include <iostream> // For std::cout.
#include <filesystem> // For std::filesystem::path.
#include <nx/sdk/analytics/helpers/plugin.h>
#include <nx/sdk/analytics/helpers/engine.h>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>
#include <nx/sdk/i_device_info.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/helpers/string.h>
#include <nx/sdk/helpers/error.h>
#include <nx/sdk/helpers/settings_response.h>


namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

// Engine::Engine(const std::filesystem::path& pluginHomeDir):
//     nx::sdk::analytics::Engine(/*enableOutput*/ true),
//     m_pluginHomeDir(pluginHomeDir)
// {
// }

// Engine::~Engine()
// {
//     //Cleanup code
// }

// void Engine::doObtainDeviceAgent(nx::sdk::Result<nx::sdk::analytics::IDeviceAgent*>* outResult, const nx::sdk::IDeviceInfo* deviceInfo)
// {
//     *outResult = new DeviceAgent(deviceInfo, m_pluginHomeDir);
// }

// void Engine::processColorDetection(const std::string& colorMessage)
// {
//     nlohmann::json message = nlohmann::json::parse(colorMessage);
//     Detection detection;
//     std::string label = message["label"];
//     float confidence = message["confidence"];
//     std::string topColors = message["top_colors"];
//     std::string pantsColors = message["pants_colors"];
//     std::string dominantColors = message["dominant_colors"];

//     if (label == "person") {
//         std::cout << "Detected person with top color: " << topColors << " and pants color: " << pantsColors << std::endl;
//     } else {
//         std::cout << "Detected " << label << " with dominant color: " << dominantColors << std::endl;
//     }
//     m_detections.push_back(detection);
// }

// std::string Engine::manifestString() const
// {
//     return /*suppress newline*/ 1 + R"json(
//     {
//         "id": "sample.opencv_object_detection",
//         "name": "OpenCV object detection",
//         "description": "This plugin is for object detection and color detection using OpenCV.",
//         "version": "1.1.0",
//         "vendor": "Sample Inc."
//     }
//     )json";
// }


Engine::Engine():
    // Call the DeviceAgent helper class constructor telling it to verbosely report to stderr.
    nx::sdk::analytics::Engine(/*enableOutput*/ true)
{ 
	this->m_minConfidence=0;

    AppLogger::clear();
	AppLogger::debug("Plugin engine started", true);
}

Engine::~Engine()
{
	AppLogger::debug("Plugin engine closed", true);
}

/**
 * Called when the Server opens a video-connection to the camera if the plugin is enabled for this
 * camera.
 *
 * @param outResult The pointer to the structure which needs to be filled with the resulting value
 *     or the error information.
 * @param deviceInfo Contains various information about the related device such as its id, vendor,
 *     model, etc.
 */

void Engine::showEngineMessage(std::string header, std::string text, nx::sdk::IPluginDiagnosticEvent::Level level)
{  AppLogger::debug("Show engine message called", true);
    if (this->m_diagnosticsEnabled == true)
    {
        pushPluginDiagnosticEvent(
            level,
            header,
            text
        );
    }
}

nx::sdk::Result<const nx::sdk::ISettingsResponse*> Engine::settingsReceived()
{ AppLogger::debug("Setting recived called in engine", true);
    auto settingsResponse = new SettingsResponse();
    try
    {
        showEngineMessage(
            "Engine settings",
            "Get Settings",
            nx::sdk::IPluginDiagnosticEvent::Level::info
        );

        std::string debug = settingValue(kDetailedLogs);
        bool detailedLogs = (debug=="true"||debug=="1");

        std::string diagnostic = settingValue(kThrowPluginDiagnostics);
        this->m_diagnosticsEnabled = (diagnostic == "true" || diagnostic == "1");

        ss.clear();
        std::string conf = settingValue(kMinConfidence);
        ss << conf;
        ss >> this->m_minConfidence;

        
        this->m_kafkaIP = settingValue(kKafkaIP);
        this->m_kafkaPort = settingValue(kKafkaPort);

    }
    catch(const std::exception& e)
    {
        std::stringstream ss;
        ss << "Engine settings error: " << e.what() << '\n';
        AppLogger::debug(ss.str());
        showEngineMessage(
            "Engine error",
            ss.str(),
            nx::sdk::IPluginDiagnosticEvent::Level::error
        );
    }
    AppLogger::debug("Setting recived in engine passed", true);
    return settingsResponse;
}


void Engine::doObtainDeviceAgent(Result<IDeviceAgent*>* outResult, const IDeviceInfo* deviceInfo)
{  AppLogger::debug("do obtein device agent in engine", true);
    // Create a new instance of DeviceAgent with appropriate arguments
    *outResult = new DeviceAgent(
        deviceInfo,
        this->m_minConfidence,
        this->m_kafkaPort,
        this->m_kafkaIP
    );


}
/**
 * @return JSON with the particular structure. Note that it is possible to fill in the values
 *     that are not known at compile time, but should not depend on the Engine settings.
 */

std::string Engine::manifestString() const
	{
		// NOT for OpenCV: Ask the Server to supply uncompressed video frames in YUV420 format (see
		// https://en.wikipedia.org/wiki/YUV).
		//
		// Note that this format is used internally by the Server, therefore requires minimum
		// resources for decoding, thus it is the recommended format.

		// Ask the Server to supply uncompressed video frames in BGR format, as it is native format for
		// OpenCV.
		return /*suppress newline*/ 1 + (const char*)R"json(
		{
			"capabilities": "needUncompressedVideoFrames_bgr",
			"preferredStream":"primary",
            "deviceAgentSettingsModel": {
                "type": "Settings",
		        "items": [
                    {
                        "type": "GroupBox",
                        "caption": "Device Settings",
                        "items": [
                                
                                {
                                    "type": "SwitchButton",
                                    "name": ")json" + kPersonEnabled + R"json(",
                                    "caption": "Person Detection enabled?",
                                    "description": "If true, person enabled",
                                    "defaultValue": true
                                },
                                {
                                    "type": "SwitchButton",
                                    "name": ")json" + kColorEnabled + R"json(",
                                    "caption": "Gender enabled?",
                                    "description": "If true, color enabled",
                                    "defaultValue": true
                                },
                                {
                                    "type": "SwitchButton",
                                    "name": ")json" + kMaskEnabled + R"json(",
                                    "caption": "Mask enabled?",
                                    "description": "If true, mask enabled",
                                    "defaultValue": true
                                },
                                {
                                    "type": "SwitchButton",
                                    "name": ")json" + kPOIMatchEnabled + R"json(",
                                    "caption": "POI Matching enabled?",
                                    "description": "If true, POI enabled",
                                    "defaultValue": true
                                }
                    
                            ]
                            
                    }
                ]
            }
	
		}
		
		)json";

	} //manifest


} // namespace opencv_object_detection
} // namespace vms_server_plugins
} // namespace sample_company
} // namespace nx