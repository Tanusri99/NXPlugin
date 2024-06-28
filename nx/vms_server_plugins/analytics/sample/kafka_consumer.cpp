#include "kafka_consumer.h"
#include <iostream>

KafkaConsumer::KafkaConsumer(const std::string& brokers, const std::string& topic)
    : m_brokers(brokers), m_topic(topic), m_running(false), m_consumer(nullptr), m_conf(nullptr), m_tconf(nullptr)
{
    m_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    m_tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

    std::string errstr;
    m_conf->set("metadata.broker.list", m_brokers, errstr);

    m_consumer = RdKafka::KafkaConsumer::create(m_conf, errstr);
    if (!m_consumer) {
        std::cerr << "Failed to create consumer: " << errstr << std::endl;
    }

    m_consumer->subscribe({m_topic});
}

KafkaConsumer::~KafkaConsumer()
{
    m_running = false;
    if (m_consumerThread.joinable())
        m_consumerThread.join();
    if (m_consumer)
        m_consumer->close();
    delete m_consumer;
    delete m_conf;
    delete m_tconf;
}

void KafkaConsumer::setMessageCallback(std::function<void(const std::string&)> callback)
{
    m_messageCallback = callback;
}

void KafkaConsumer::start()
{
    m_running = true;
    m_consumerThread = std::thread([this] {
        while (m_running) {
            std::string message = this->consume();
            if (!message.empty() && m_messageCallback) {
                m_messageCallback(message);
            }
        }
    });
}

std::string KafkaConsumer::consume()
{
    std::string message_str;
    RdKafka::Message* message = m_consumer->consume(1000);
    if (message->err() == RdKafka::ERR_NO_ERROR) {
        message_str = std::string(static_cast<const char*>(message->payload()), message->len());
    }
    delete message;
    return message_str;
}
