// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"
#include "engine.h"
#include <chrono>
#include <exception>
#include <cctype>
#include <iostream>
#include <thread>
#include <filesystem>

#include <nx/sdk/analytics/helpers/event_metadata.h>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/analytics/helpers/plugin.h>
#include <nx/sdk/analytics/helpers/engine.h>
#include <nx/sdk/helpers/string.h>
#include <nx/sdk/i_device_info.h>
#include <nx/sdk/helpers/string_map.h>
#include <nx/sdk/helpers/settings_response.h>
#include <nx/sdk/helpers/error.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/kit/json.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include "kafka_consumer.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {

using namespace nx::sdk;
using namespace nx::sdk::analytics;
using namespace std::string_literals;

/**
 * Called when the Server opens a video-connection to the camera if the plugin is enabled for this
 * camera.
 *
 * @param outResult The pointer to the structure which needs to be filled with the resulting value
 *     or the error information.
 * @param deviceInfo Contains various information about the related device such as its id, vendor,
 *     model, etc.
 */

DeviceAgent::DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo, const std::filesystem::path& pluginHomeDir): 
    ConsumingDeviceAgent(deviceInfo, true),
    m_terminated(false), //Initialize the member variable
    m_kafkaConsumer("localhost:9092", "color_detection"),
    m_nxCamId(deviceInfo->id()) //Initialize the camera ID
{
    // Extract nxCamId from deviceInfo
    //m_nxCamId = deviceInfo->id();
    m_nxCamId.erase(std::remove(m_nxCamId.begin(), m_nxCamId.end(), '{'), m_nxCamId.end());
    m_nxCamId.erase(std::remove(m_nxCamId.begin(), m_nxCamId.end(), '}'), m_nxCamId.end());

    m_kafkaConsumer.setMessageCallback([this](const std::string& message) {
        processKafkaMessage(message);
    });

    m_kafkaConsumerThread = std::thread([this] {
        m_kafkaConsumer.start();
    });
}

DeviceAgent::~DeviceAgent()
{
    m_terminated = true; //Set the flag to terminate the thread
    if (m_kafkaConsumerThread.joinable())
        m_kafkaConsumerThread.join();
}

std::string DeviceAgent::getNxCamId() const
{
    return m_nxCamId; //Return the camera ID
}

std::string DeviceAgent::manifestString() const
{
    return /*suppress newline*/ 1 + R"json(
{
    "eventTypes": [],
    "supportedTypes": [
        {
            "objectTypeId": ")json" + kPersonObjectType + R"json("
        }
    ]
}
)json";
}

bool DeviceAgent::pushUncompressedVideoFrame(const IUncompressedVideoFrame* videoFrame)
{
    if (m_terminated)
        return true;

    const MetadataPacketList metadataPackets = processFrame(videoFrame);
    for (const Ptr<IMetadataPacket>& metadataPacket: metadataPackets)
    {
        metadataPacket->addRef();
        pushMetadataPacket(metadataPacket.get());
    }

    return true;
}

void DeviceAgent::doSetNeededMetadataTypes(
    nx::sdk::Result<void>* outValue,
    const nx::sdk::analytics::IMetadataTypes* /*neededMetadataTypes*/)
{
    if (m_terminated)
        return;

    m_kafkaConsumer.start();
}

void DeviceAgent::processKafkaMessage(const std::string& message)
{
    auto messageJson = nlohmann::json::parse(message);
    std::string label = messageJson["label"];
    float confidence = messageJson["confidence"];
    std::string top_colors = messageJson["top_colors"];
    std::string pants_colors = messageJson["pants_colors"];
    std::string dominant_colors = messageJson["dominant_colors"];

    // Handle the detection results based on your logic
    if (label == "person")
    {
        // Process top and pants color detection
        std::cout << "Detected person with top color: " << top_colors <<
            " and pants color: " << pants_colors << std::endl;
    }
    else
    {
        // Process other detections
        std::cout << "Detected " << label << " with dominant color: " << dominant_colors << std::endl;
    }
}

DeviceAgent::MetadataPacketList DeviceAgent::processFrame(
    const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame)
{
    // Fetch actual detections from Kafka
    std::vector<Detection> detections = fetchDetectionsFromKafka();
    const auto& objectMetadataPacket = detectionsToObjectMetadataPacket(detections, 0);

    MetadataPacketList result;
    if (objectMetadataPacket)
        result.push_back(objectMetadataPacket);

    return result;
}

std::vector<Detection> DeviceAgent::fetchDetectionsFromKafka()
{
    std::vector<Detection> detections;
    std::string message = m_kafkaConsumer.consume();
    if (!message.empty()) {
        nlohmann::json messageJson = nlohmann::json::parse(message);
        Detection detection;
        detection.label = messageJson["label"];
        detection.confidence = messageJson["confidence"];
        detection.topColor = messageJson["top_colors"];
        detection.pantsColor = messageJson["pants_colors"];
        detection.dominantColors = messageJson["dominant_colors"];
        detections.push_back(detection);
    }
    return detections;
}

Ptr<ObjectMetadataPacket> DeviceAgent::detectionsToObjectMetadataPacket(
    const std::vector<Detection>& detections,
    int64_t timestampUs)
{
    if (detections.empty())
        return nullptr;

    const auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();

    for (const Detection& detection: detections)
    {
        const auto objectMetadata = makePtr<ObjectMetadata>();

        // Use detection data to set metadata
        objectMetadata->setTypeId(kPersonObjectType);
        objectMetadata->setConfidence(detection.confidence);
        objectMetadataPacket->addItem(objectMetadata.get());
    }
    objectMetadataPacket->setTimestampUs(timestampUs);

    return objectMetadataPacket;
}

} 
} 
} 
}