// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"
#include "plugin_settings.h"
#include "engine.h"
#include "applogger.h"
#include "plugin.h"

#include <curl/curl.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <map>
#include <ctime>
#include <iomanip>
#include <string>
#include <cmath>
#include <sstream>
#include <algorithm>
#include "cppkafka/consumer.h"
#include "cppkafka/configuration.h"
#include <cctype>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include <nx/sdk/analytics/helpers/event_metadata.h>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/helpers/string.h>
#include <nx/sdk/helpers/string_map.h>
#include <nx/sdk/helpers/settings_response.h>
#include <nx/sdk/helpers/error.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/kit/json.h>

#include "kafka_consumer.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aol_color_detection {

using namespace nx::sdk;
using namespace nx::sdk::analytics;
using namespace std::string_literals;
using NXPoint = nx::sdk::analytics::Point;

using cppkafka::Consumer;
using cppkafka::Configuration;
using cppkafka::Message;
using cppkafka::TopicPartitionList;

/**
 * Called when the Server opens a video-connection to the camera if the plugin is enabled for this
 * camera.
 *
 * @param outResult The pointer to the structure which needs to be filled with the resulting value
 *     or the error information.
 * @param deviceInfo Contains various information about the related device such as its id, vendor,
 *     model, etc.
 */

DeviceAgent::DeviceAgent(
    const nx::sdk::IDeviceInfo* deviceInfo,
    uint64_t framePeriod,
    uint64_t durationUs,
    size_t confidence,
    std::string kafkaPort,
    std::string kafkaIP
    ): 
    ConsumingDeviceAgent(deviceInfo, true)
{
    this->m_framePeriod = framePeriod;
    this->m_duration = durationUs;
    this->m_minConfidence = confidence;
    this->m_kafkaPort = kafkaPort;
    this->m_kafkaIP = kafkaIP;
    m_heartbeatCount=0;
    this->m_heartbeatPeriod=30;

    std::string nxID = deviceInfo->id();
    this->removeBrackets(nxID, this->m_nxCamId); 
    this->m_pServerThread=NULL;
    
    serverOnline->store(true);
    t1 = new std::thread([this]() {startServer();} );
    AppLogger::debug("device agent started at:" + nxID);
  
}

DeviceAgent::~DeviceAgent()
{
    serverOnline->store(false);
    if (t1 != nullptr)
    {
        t1->join();
        delete t1;
    }
    
    std::string jsonstr = "{\"camera_id\":\"";
    jsonstr += this->m_deviceID;
    jsonstr += "\",\"analytic_type\":\"";
    jsonstr += "count";
    jsonstr += "\"}";
    std::string endpoint = "stoptevents?nxcam_id="+this->m_nxCamId; 
    std::string res = postCURL(jsonstr, endpoint);
    AppLogger::debug("device agent closed at:" + nxID);
}

void DeviceAgent::showDeviceMessage(std::string header, 
std::string text, 
nx::sdk::IPluginDiagnosticEvent::Level level)
{  AppLogger::debug("show device message");
    if (this->m_diagnosticsEnabled == true)
    {
        pushPluginDiagnosticEvent(
            level,
            header,
            text
        );
    }
}


// post config data to external server
std::string DeviceAgent::postCURL(const std::string &jsonstr, std::string endpoint)
{  AppLogger::debug("post curl started");
    std::string response;
    CURL *curl;
    struct curl_slist *slist1;

    std::string host = "http://localhost";
    std::string port = "47760";
    std::string url = host + ":" + port + "/" + endpoint;
    
    // dynamic flask server address from plugin setting
    //std::string url = this->m_hailoServerAddress + "/" + endpoint;
    // std::string url ="http://" + this->m_ColorIP + ":" + this->m_ColorPort + "/" + endpoint;
    AppLogger::debug("flask url : " + url);

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "Content-Type: application/json");

    // initialse curl instance
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 0=ignore
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonstr.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.38.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    // get response
    response = std::to_string(curl_easy_perform(curl));

    // curl clean up
    curl_easy_cleanup(curl);
    curl = NULL;
    curl_slist_free_all(slist1);
    slist1 = NULL;
    AppLogger::debug("post curl completed");
    return response;
}

void DeviceAgent::removeBrackets(std::string strIn, std::string& strOut)
{  AppLogger::debug("Remove brackets ");
    strOut.clear();
    for(auto ch: strIn){
        if(ch!='{' && ch!='}'){
            strOut.push_back(ch);
        }
    }
}

void DeviceAgent::startHeartbeatThread() {
    
    // Start a new thread to execute the heartbeat logic
    std::thread heartbeatThread([this]() {
        try {

            std::string brokers = this->m_kafkaIP+ ":"+ this->m_kafkaPort;
            std::string kafka_topic = this->m_nxCamId;
            std::string group_id = "Color-Plugin";
            std::string jsonstr = "{\"kafka_broker\":\"";
            jsonstr += brokers;
            jsonstr += "\"kafka_topic\":\"";
            jsonstr += kafka_topic;
            jsonstr += "\",\"camera_id\":\"";
            jsonstr += this->m_nxCamId;
            jsonstr += "\",\"group_id\":\"";
            jsonstr += group_id;
            jsonstr += "\"}";
            if (m_heartbeatCount % m_heartbeatPeriod == 0) {
                AppLogger::debug("Send heartbeat");
                std::string endpoint = "heartbeat?nxcam_id=" + m_nxCamId;
                std::string res = postCURL(jsonstr, endpoint);
                AppLogger::debug("Response from flask server: " + res);
            }
            m_heartbeatCount++;
        } catch (const std::exception& e) {
            std::stringstream ss;
            ss << "Heartbeat error: " << e.what() << '\n';
            AppLogger::debug(ss.str());
            showDeviceMessage(
                "Heartbeat Error",
                ss.str(),
                nx::sdk::IPluginDiagnosticEvent::Level::error
            );
        }
    });

    // Detach the thread so it can run independently
    heartbeatThread.detach();
}


void DeviceAgent::startServer()
{
    std::shared_ptr<std::atomic<bool>> online(this->serverOnline);

    // TODO: Change broker to pull from Settings page instead of hardcoded address
    std::string brokers = this->m_kafkaIP+ ":"+ this->m_kafkaPort;
    std::string kafka_topic = this->m_nxCamId;
    std::string group_id = "Color-Plugin";

    // Construct the configuration
    Configuration config = {
        { "metadata.broker.list", brokers },
        { "group.id", group_id },
        // Disable auto commit
        { "enable.auto.commit", false }
    };

    // json string format data to be sent to flask app (external) via curl 
    std::string jsonstr = "{\"kafka_broker\":\"";
    jsonstr += brokers;
    jsonstr += "\"kafka_topic\":\"";
    jsonstr += kafka_topic;
    jsonstr += "\",\"camera_id\":\"";
    jsonstr += this->m_nxCamId;
    jsonstr += "\",\"group_id\":\"";
    jsonstr += group_id;
    jsonstr += "\"}";
  
    AppLogger::debug("Jsonstr data: " + jsonstr);

    // inital send
    std::string endpoint = "startevents?nxcam_id="+this->m_nxCamId;
    std::string res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
    AppLogger::debug("Response from flask server: " + res);

    // check if send successful and continue to retry if not
    while(res != "0" && online->load() && online.use_count() == 2)
    {
        sleep(5);
        res = postCURL(jsonstr, endpoint); // send topic + cam info via curl
        AppLogger::debug("Response from flask server: " + res);
    }

    // Create the consumer
    Consumer consumer(config);
    consumer.subscribe({ kafka_topic });
    AppLogger::debug("Subscribed to topic -> " + kafka_topic);
    while (online->load() && online.use_count() == 2)
    {  //AppLogger::debug("Inside while loop");     
        try
        {
           Message msg = consumer.poll();

           AppLogger::debug("After consumer pool");
            if(msg)
            {
                // If we managed to get a message
                if (msg.get_error()) {
                    AppLogger::debug("Message error");
                    // Ignore EOF notifications from rdkafka
                    if (!msg.is_eof()) {
                        //Applogger::debug("[Kafka] Received error notification: " + msg.get_error());
                        AppLogger::debug("Message error next");
                    }
                }
                else {
                    // Print the key (if any)
                    // if (msg.get_key()) {
                    //     cout << msg.get_key() << " -> ";
                    // }
                    // Print the payload
                    AppLogger::debug("No Message error");
                    std::string payload = msg.get_payload();
                    AppLogger::debug("Jsonstr data from heartbeat_camera: " + payload);
                    json data = json::parse(payload);
                    


                    createObject(data);

                    // Now commit the message
                    consumer.commit(msg);
                }
            }
            else{
                AppLogger::debug("No Message found");
            }
            std::thread heartThread(&DeviceAgent::startHeartbeatThread, this);
             heartThread.detach();
        }
        catch (const std::exception& e)
        {
                // Create a stringstream variable
            std::stringstream ss;
            // Some code to populate ss with relevant information
            ss << "Additional information: " << e.what();
            // Log the error message and details
            AppLogger::debug("Error in consuming kafka data");
            this->showDeviceMessage(
            "Kafka data consuming Error",
            ss.str(),
            nx::sdk::IPluginDiagnosticEvent::Level::error
            );

        }
    }
    return;
}

  
std::unordered_map<std::string, nx::sdk::Uuid> objectIdToUuidMap;    
void DeviceAgent::createObject(json data)
{   AppLogger::debug("Create object started");

     try
        
        { 
        json detection_data = data["detectionData"][0];

        if (detection_data.contains("appearanceID") && !detection_data["appearanceID"].is_null()) {
            appearance_id = (detection_data["appearanceID"]);
            nx::sdk::Uuid uuid;
            if (objectIdToUuidMap.find(appearance_id) == objectIdToUuidMap.end()) {
                uuid = nx::sdk::UuidHelper::randomUuid();
                objectIdToUuidMap[appearance_id] = uuid;
            }
            else {
                uuid = objectIdToUuidMap[appearance_id];
            }
            m_trackId = uuid;

        }
        if (detection_data.contains("poiMatchOutcome")) {
            poi_match_outcome = detection_data["poiMatchOutcome"];
        }

        if (detection_data.contains("poiName")&& !detection_data["poiName"].is_null()) {
            poi_name = detection_data["poiName"];
        }

        if (detection_data.contains("poiConfidence")&& !detection_data["poiConfidence"].is_null()) {
            poi_confidence = detection_data["poiConfidence"];
        }
        if (detection_data.contains("frameWidth") && !detection_data["frameWidth"].is_null()) {
            frame_width = detection_data["frameWidth"];
        }

        if (detection_data.contains("frameHeight")&& !detection_data["frameHeight"].is_null()) {
            frame_height = detection_data["frameHeight"];
        }

        if (detection_data.contains("detector_score")&& !detection_data["detector_score"].is_null()) {
            detector_score = detection_data["detector_score"];
        }

        if (detection_data.contains("timestampMicrosecond")&& !detection_data["timestampMicrosecond"].is_null()) {
            m_lastVideoFrameTimestampUs = detection_data["timestampMicrosecond"];
        }

        if (detection_data.contains("x1")&& !detection_data["x1"].is_null()) {
            x1 = detection_data["x1"];
        }

        if (detection_data.contains("y1")&& !detection_data["y1"].is_null()) {
            y1 = detection_data["y1"];
        }

        if (detection_data.contains("x2")&& !detection_data["x2"].is_null()) {
            x2 = detection_data["x2"];
        }

        if (detection_data.contains("y2")&& !detection_data["y2"].is_null()) {
            y2 = detection_data["y2"];
        }
         
         }
        catch (const std::exception& e)
        {
            //respose code from curl post
            std::string err1 = e.what();
            AppLogger::debug("Error in json data");
        }
        
    this->m_xMin= (x1/frame_width);
    this->m_yMin=(y1/frame_height); 
    this->m_xMax=(x2/frame_width);
    this->m_yMax=(y2/frame_height);
    this->m_width=(m_yMax-m_yMin);
    this->m_height=(m_xMax-m_xMin);

    auto eventMetadataPacket = generateEventMetadataPacket();
    if (eventMetadataPacket)
    {
        // Send generated metadata packet to the Server.
        pushMetadataPacket(eventMetadataPacket.releasePtr());
    }

    auto objectMetadataPacket = generateObjectMetadataPacket();
    if (objectMetadataPacket)
    {
        pushMetadataPacket(objectMetadataPacket.releasePtr());
    }
    AppLogger::debug("Create object completed");
}


Result<const ISettingsResponse*> DeviceAgent::settingsReceived()
{
    const auto settingsResponse = new sdk::SettingsResponse();
    try
    {
        
        //Applogger::debug("Device Settings");
        this->showDeviceMessage(
            "Device Agent",
            "Device Settings",
            nx::sdk::IPluginDiagnosticEvent::Level::info
        );
        
    }
    catch(const std::exception& e)
    {
        std::stringstream ss;
        ss << "Device settings error: " << e.what() << '\n';
         //Applogger::debug(ss.str());
        //this->m_terminated=true;
    }
    
    return settingsResponse;
}



/**
 *  @return JSON with the particular structure. Note that it is possible to fill in the values
 * that are not known at compile time, but should not depend on the DeviceAgent settings.
 */
std::string DeviceAgent::manifestString() const
{
    // Tell the Server that the plugin can generate the events and objects of certain types.
    // Id values are strings and should be unique. Format of ids:
    // `{vendor_id}.{plugin_id}.{event_type_id/object_type_id}`.
    //
    // See the plugin manifest for the explanation of vendor_id and plugin_id.
    return /*suppress newline*/ 1 + (const char*) R"json(

{
"eventTypes": [
    {
    "id": ")json" + kPersonDetected + R"json(",  
        "name": "Person Found"
    },
    {
    "id": ")json" + kPersonNoDetected + R"json(",
        "name": "Person Not Found "
    }
    ],
    "objectTypes": [
    {
    "id": ")json" + kColorDetected + R"json(",
    "name": "Color Detected"
    },
    {
    "id": ")json" + kColorNoDetected + R"json(",
    "name": "Color Not Detected"
    }
    ]
}
)json";
}


/**setTrackId
 * Called when the Server sends a new uncompressed frame from a camera.
 */
bool DeviceAgent::pushUncompressedVideoFrame(const IUncompressedVideoFrame* videoFrame)
{    
   
    return true; //< There were no errors while processing the video frame.
}

/**
 * Serves the similar purpose as pushMetadataPacket(). The differences are:
 * - pushMetadataPacket() is called by the plugin, while pullMetadataPackets() is called by Server.
 * - pushMetadataPacket() expects one metadata packet, while pullMetadataPacket expects the
 *     std::vector of them.
 *
 * There are no strict rules for deciding which method is "better". A rule of thumb is to use
 * pushMetadataPacket() when you generate one metadata packet and do not want to store it in the
 * class field, and use pullMetadataPackets otherwise.
 */
bool DeviceAgent::pullMetadataPackets(std::vector<IMetadataPacket*>* metadataPackets)
{
    return true; //< There were no errors while filling metadataPackets.
}

void DeviceAgent::doSetNeededMetadataTypes(
    nx::sdk::Result<void>* /*outValue*/,
    const nx::sdk::analytics::IMetadataTypes* /*neededMetadataTypes*/)
{
}

//-------------------------------------------------------------------------------------------------
// private

Ptr<IMetadataPacket> DeviceAgent::generateEventMetadataPacket()
{
  
    // EventMetadataPacket contains arbitrary number of EventMetadata.
    const auto eventMetadataPacket = makePtr<EventMetadataPacket>();
    // Bind event metadata packet to the last video frame using a timestamp.
    eventMetadataPacket->setTimestampUs(m_lastVideoFrameTimestampUs);
    // Zero duration means that the event is not sustained, but momental.
    eventMetadataPacket->setDurationUs(0);

    // EventMetadata contains an information about event.
    const auto eventMetadata = makePtr<EventMetadata>();
    try
    {
   
    if(poi_match_outcome=="matched"){
        eventMetadata->setTypeId(kPersonDetected);
    }
    else{
        eventMetadata->setTypeId(kPersonNoDetected);   
    }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    // Set all required fields.
    eventMetadata->setIsActive(true);
    eventMetadata->setCaption("Person detected");
    eventMetadata->setDescription("New Person deteccted");

    eventMetadataPacket->addItem(eventMetadata.get());
    return eventMetadataPacket;
}

// generate object type detected from hailo
//TODO:

Ptr<IMetadataPacket> DeviceAgent::generateObjectMetadataPacket()
{
    // ObjectMetadataPacket contains arbitrary number of ObjectMetadata.
    const auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();

    // Bind the object metadata to the last video frame using a timestamp.
    objectMetadataPacket->setTimestampUs(m_lastVideoFrameTimestampUs);
    objectMetadataPacket->setDurationUs(0);

    // ObjectMetadata contains information about an object on the frame.
    const auto objectMetadata = makePtr<ObjectMetadata>();
    const std::string typeId = poi_match_outcome;
    try
    {    
        if(typeId=="matched"){
        objectMetadata->setTypeId(kColorDetected);
        float confidence = poi_confidence;
        }
        else{
        objectMetadata->setTypeId(kColorNoDetected);
        float confidence= detector_score * 100;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        AppLogger::debug("Error on object type");
    }
  
    objectMetadata->setTrackId(this->m_trackId);

  
    //objectMetadata->setSubtype(this->m_people_count_type);    

    double width = m_width;
    double height = m_height;
    double x = m_xMin;
    double y = m_yMin;

    objectMetadata->setBoundingBox(Rect(x, y, width, height));

    
    /*objectMetadata->addAttribute(makePtr<Attribute>("CurrentTime", []() {
                    auto now = std::chrono::system_clock::now();
                    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
                    std::stringstream ss;
                    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");
                    return ss.str();
                    }()));*/
    if(this->m_POIEnabled){
        if(typeId == "matched"){
            // POI event data
            
            objectMetadata->addAttribute(makePtr<Attribute>(IAttribute::Type::string, "Name", poi_name)); 
            objectMetadata->addAttribute(makePtr<Attribute>(IAttribute::Type::string, "Confidence", std::to_string(poi_confidence)));
           //objectMetadata->addAttribute(makePtr<Attribute>(IAttribute::Type::string, "Watchlist color", watchlist_color));
            
        }
    }

    objectMetadataPacket->addItem(objectMetadata.get());

    

    return objectMetadataPacket;
}

//     m_terminated(false), //Initialize the member variable
//     m_kafkaConsumer("localhost:9092", "color_detection"),
//     m_nxCamId(deviceInfo->id()) //Initialize the camera ID
// {
//     // Extract nxCamId from deviceInfo
//     //m_nxCamId = deviceInfo->id();
//     m_nxCamId.erase(std::remove(m_nxCamId.begin(), m_nxCamId.end(), '{'), m_nxCamId.end());
//     m_nxCamId.erase(std::remove(m_nxCamId.begin(), m_nxCamId.end(), '}'), m_nxCamId.end());

//     m_kafkaConsumer.setMessageCallback([this](const std::string& message) {
//         processKafkaMessage(message);
//     });

//     m_kafkaConsumerThread = std::thread([this] {
//         m_kafkaConsumer.start();
//     });
// }

// DeviceAgent::~DeviceAgent()
// {
//     m_terminated = true; //Set the flag to terminate the thread
//     if (m_kafkaConsumerThread.joinable())
//         m_kafkaConsumerThread.join();
// }

// std::string DeviceAgent::getNxCamId() const
// {
//     return m_nxCamId; //Return the camera ID
// }

// std::string DeviceAgent::manifestString() const
// {
//     return /*suppress newline*/ 1 + R"json(
// {
//     "eventTypes": [],
//     "supportedTypes": [
//         {
//             "objectTypeId": ")json" + kPersonObjectType + R"json("
//         }
//     ]
// }
// )json";
// }

// bool DeviceAgent::pushUncompressedVideoFrame(const IUncompressedVideoFrame* videoFrame)
// {
//     if (m_terminated)
//         return true;

//     const MetadataPacketList metadataPackets = processFrame(videoFrame);
//     for (const Ptr<IMetadataPacket>& metadataPacket: metadataPackets)
//     {
//         metadataPacket->addRef();
//         pushMetadataPacket(metadataPacket.get());
//     }

//     return true;
// }

// void DeviceAgent::doSetNeededMetadataTypes(
//     nx::sdk::Result<void>* outValue,
//     const nx::sdk::analytics::IMetadataTypes* /*neededMetadataTypes*/)
// {
//     if (m_terminated)
//         return;

//     m_kafkaConsumer.start();
// }

// void DeviceAgent::processKafkaMessage(const std::string& message)
// {
//     auto messageJson = nlohmann::json::parse(message);
//     std::string label = messageJson["label"];
//     float confidence = messageJson["confidence"];
//     std::string top_colors = messageJson["top_colors"];
//     std::string pants_colors = messageJson["pants_colors"];
//     std::string dominant_colors = messageJson["dominant_colors"];

//     // Handle the detection results based on your logic
//     if (label == "person")
//     {
//         // Process top and pants color detection
//         std::cout << "Detected person with top color: " << top_colors <<
//             " and pants color: " << pants_colors << std::endl;
//     }
//     else
//     {
//         // Process other detections
//         std::cout << "Detected " << label << " with dominant color: " << dominant_colors << std::endl;
//     }
// }

// DeviceAgent::MetadataPacketList DeviceAgent::processFrame(
//     const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame)
// {
//     // Fetch actual detections from Kafka
//     std::vector<Detection> detections = fetchDetectionsFromKafka();
//     const auto& objectMetadataPacket = detectionsToObjectMetadataPacket(detections, 0);

//     MetadataPacketList result;
//     if (objectMetadataPacket)
//         result.push_back(objectMetadataPacket);

//     return result;
// }

// std::vector<Detection> DeviceAgent::fetchDetectionsFromKafka()
// {
//     std::vector<Detection> detections;
//     std::string message = m_kafkaConsumer.consume();
//     if (!message.empty()) {
//         nlohmann::json messageJson = nlohmann::json::parse(message);
//         Detection detection;
//         detection.label = messageJson["label"];
//         detection.confidence = messageJson["confidence"];
//         detection.topColor = messageJson["top_colors"];
//         detection.pantsColor = messageJson["pants_colors"];
//         detection.dominantColors = messageJson["dominant_colors"];
//         detections.push_back(detection);
//     }
//     return detections;
// }

// Ptr<ObjectMetadataPacket> DeviceAgent::detectionsToObjectMetadataPacket(
//     const std::vector<Detection>& detections,
//     int64_t timestampUs)
// {
//     if (detections.empty())
//         return nullptr;

//     const auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();

//     for (const Detection& detection: detections)
//     {
//         const auto objectMetadata = makePtr<ObjectMetadata>();

//         // Use detection data to set metadata
//         objectMetadata->setTypeId(kPersonObjectType);
//         objectMetadata->setConfidence(detection.confidence);
//         objectMetadataPacket->addItem(objectMetadata.get());
//     }
//     objectMetadataPacket->setTimestampUs(timestampUs);

//     return objectMetadataPacket;
// }

} 
} 
} 
}