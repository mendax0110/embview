#include "core/TransportFactory.h"

#include "core/SerialTransport.h"
#include "core/TcpTransport.h"
#include "core/UdpTransport.h"

#include <nlohmann/json.hpp>
#include <stdexcept>

using namespace embview::core;

TransportFactory& TransportFactory::instance()
{
    static TransportFactory factory;
    return factory;
}

TransportFactory::TransportFactory()
{
    registerCreator("serial", [](const nlohmann::json& config) -> std::unique_ptr<ITransport>
    {
        SerialConfig sc;
        sc.portName = config.value("port", "");
        sc.baudRate = config.value("baud", 115200);
        sc.dataBits = config.value("dataBits", 8);
        sc.stopBits = config.value("stopBits", 1);
        sc.parity   = config.value("parity", 0);
        return std::make_unique<SerialTransport>(std::move(sc));
    });

    registerCreator("tcp", [](const nlohmann::json& config) -> std::unique_ptr<ITransport>
    {
        TcpConfig tc;
        tc.host = config.value("host", "192.168.1.1");
        tc.port = config.value("port", static_cast<uint16_t>(5000));
        return std::make_unique<TcpTransport>(std::move(tc));
    });

    registerCreator("udp", [](const nlohmann::json& config) -> std::unique_ptr<ITransport>
    {
        UdpConfig uc;
        uc.host = config.value("host", "0.0.0.0");
        uc.port = config.value("port", static_cast<uint16_t>(5000));
        uc.broadcast = config.value("broadcast", false);
        return std::make_unique<UdpTransport>(std::move(uc));
    });
}

void TransportFactory::registerCreator(const std::string& type, Creator creator)
{
    m_creators[type] = std::move(creator);
}

std::unique_ptr<ITransport> TransportFactory::create(const std::string& type, const nlohmann::json& config) const
{
    auto it = m_creators.find(type);
    if (it == m_creators.end())
    {
        throw std::runtime_error("Unknown transport type: " + type);
    }
    return it->second(config);
}

bool TransportFactory::hasType(const std::string& type) const
{
    return m_creators.contains(type);
}
