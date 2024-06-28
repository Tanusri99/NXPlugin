// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <vector>
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
#include "detection.h"

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
public:
    using MetadataPacketList = std::vector<nx::sdk::Ptr<nx::sdk::analytics::IMetadataPacket>>;

public:
    DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo, const std::filesystem::path& pluginHomeDir);
    ~DeviceAgent();

    std::string getNxCamId() const;

protected:
    virtual std::string manifestString() const override;

    virtual bool pushUncompressedVideoFrame(
        const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame) override;

    virtual void doSetNeededMetadataTypes(
        nx::sdk::Result<void>* outValue,
        const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes) override;

private:
    bool m_terminated = false; //Initialize the member variable
    void processKafkaMessage(const std::string& message);
    MetadataPacketList processFrame(const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame);
    std::vector<Detection> fetchDetectionsFromKafka();

    nx::sdk::Ptr<nx::sdk::analytics::ObjectMetadataPacket> detectionsToObjectMetadataPacket(
        const std::vector<Detection>& detections,
        int64_t timestampUs);

private:
    const std::string kPersonObjectType = "nx.base.Person";

    bool m_running = true;
    KafkaConsumer m_kafkaConsumer;
    std::thread m_kafkaConsumerThread;
    std::string m_nxCamId;
};

} // namespace opencv_object_detection
} // namespace vms_server_plugins
} // namespace sample_company
} // namespace nx
