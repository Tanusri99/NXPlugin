// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <vector>
#include <map>
#include <filesystem>
#include <thread>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>
#include <nx/sdk/analytics/i_metadata_types.h>
#include <nx/sdk/ptr.h>
#include <nx/sdk/i_device_info.h>
#include "kafka_consumer.h"
#include "engine.h"
#include <jsoncpp/json/json.h>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {

using namespace nx::sdk;
using namespace nx::sdk::analytics;
/**
 * Called when the Server opens a video-connection to the camera if the plugin is enabled for this
 * camera.
 *
 * @param outResult The pointer to the structure which needs to be filled with the resulting value
 *     or the error information.
 * @param deviceInfo Contains various information about the related device such as its id, vendor,
 *     model, etc.
 */

class DeviceAgent: public nx::sdk::analytics::ConsumingDeviceAgent
{
// public:
//     using MetadataPacketList = std::vector<nx::sdk::Ptr<nx::sdk::analytics::IMetadataPacket>>;

// public:
//     DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo, const std::filesystem::path& pluginHomeDir);
//     ~DeviceAgent();

//     std::string getNxCamId() const;

// protected:
//     virtual std::string manifestString() const override;

//     virtual bool pushUncompressedVideoFrame(
//         const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame) override;

//     virtual void doSetNeededMetadataTypes(
//         nx::sdk::Result<void>* outValue,
//         const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes) override;

// private:
//     bool m_terminated = false; //Initialize the member variable
//     void processKafkaMessage(const std::string& message);
//     MetadataPacketList processFrame(const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame);
//     std::vector<Detection> fetchDetectionsFromKafka();

//     nx::sdk::Ptr<nx::sdk::analytics::ObjectMetadataPacket> detectionsToObjectMetadataPacket(
//         const std::vector<Detection>& detections,
//         int64_t timestampUs);

// private:
//     const std::string kPersonObjectType = "nx.base.Person";

//     bool m_running = true;
//     KafkaConsumer m_kafkaConsumer;
//     std::thread m_kafkaConsumerThread;
//     std::string m_nxCamId;
// };

public:
    DeviceAgent(
        const nx::sdk::IDeviceInfo* deviceInfo,
        size_t confidence,
        std::string kafkaPort,
        std::string kafkaIP
        );
    virtual ~DeviceAgent() override;
    void showDeviceMessage(std::string header, std::string text, nx::sdk::IPluginDiagnosticEvent::Level level);
    void removeBrackets(std::string strIn, std::string& strOut);

protected:
    virtual std::string manifestString() const override;

    virtual bool pushUncompressedVideoFrame(
        const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame) override;

    virtual bool pullMetadataPackets(
        std::vector<nx::sdk::analytics::IMetadataPacket*>* metadataPackets) override;

    virtual nx::sdk::Result<const nx::sdk::ISettingsResponse*> settingsReceived() override;        
   

    virtual void doSetNeededMetadataTypes(
        nx::sdk::Result<void>* outValue,
        const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes) override;

private:
    nx::sdk::Ptr<nx::sdk::analytics::IMetadataPacket> generateEventMetadataPacket();
    nx::sdk::Ptr<nx::sdk::analytics::IMetadataPacket> generateObjectMetadataPacket();

private:
    nx::sdk::Uuid m_trackId;

    /** Used for binding object and event metadata to the particular video frame. */
    int64_t m_lastVideoFrameTimestampUs = 0;
    int m_frameIndex = 0; /**< Used for generating the detection in the right place. */
    std::string m_label;
    float m_confidence;
    double m_xMin;
    double m_yMin;
    double m_xMax;
    double m_yMax;    
    double m_width;
    double m_height;
    std::string m_objectType;
    std::string appearance_id; 
    int frame_id;
    double frame_width;
    double frame_height;
    double detector_score;
    std::string m_kafkaTopic;
    std::string m_kafkaGroup;
    std::string m_kafkaBroker;
    std::string m_kafkaPort;
    std::string m_kafkaIP;
    std::string poi_match_outcome;
    std::string poi_name;
    double poi_confidence;
    double x1;
    double y1;
    double x2;
    double y2;

private:
    void startServer();
    void createObject(json data);
    size_t getClosestColorIndex(std::string hexColor);
    void startHeartbeatThread();
   
    std::map<std::string, const std::string> map;
   // void createHashMap();
    //std::string getObjectType();
    std::shared_ptr<std::atomic<bool>> serverOnline = std::make_shared<std::atomic<bool>>(true /*or true*/);
    std::thread* t1;   
    //void setEnabledObjects(std::vector<std::string> enabledObjectList);
    std::vector<std::string> m_enabledObjectList; // selected names   
    std::string postCURL(const std::string &jsonstr, std::string endpoint); // curl post    
     // flask server address
    size_t m_heartbeatCount=0;
    size_t m_heartbeatPeriod=100;
    uint64_t m_framePeriod = 0;
    uint64_t m_duration = 0;
    bool m_diagnosticsEnabled = false;
    size_t m_minConfidence=0;
    std::string m_kafkaPort;
    std::string m_kafkaIP;
    bool m_POIEnabled;
    // device info
    std::string m_nxCamId;
    std::thread* m_pServerThread;
    // heartbeat time 
    std::chrono::time_point<std::chrono::system_clock> m_heartbeat = std::chrono::system_clock::now();
    std::string m_deviceID; 
    std::string nxID;
    

};

} // namespace opencv_object_detection
} // namespace vms_server_plugins
} // namespace sample_company
} // namespace nx
