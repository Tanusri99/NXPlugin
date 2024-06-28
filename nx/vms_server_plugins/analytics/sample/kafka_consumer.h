#pragma once

#include <string>
#include <thread>
#include <functional>
#include <librdkafka/rdkafkacpp.h>
#include <nlohmann/json.hpp>

class KafkaConsumer {
public:
    KafkaConsumer(const std::string& brokers, const std::string& topic);
    ~KafkaConsumer();

    void setMessageCallback(std::function<void(const std::string&)> callback);
    void start();
    std::string consume(); // Change return type to std::string

private:
    std::string m_brokers;
    std::string m_topic;
    std::function<void(const std::string&)> m_messageCallback;
    std::thread m_consumerThread;
    bool m_running;
    RdKafka::KafkaConsumer* m_consumer;
    RdKafka::Conf* m_conf;
    RdKafka::Conf* m_tconf;
};
