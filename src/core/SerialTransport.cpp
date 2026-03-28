#include "core/SerialTransport.h"

#include <libserialport.h>
#include <spdlog/spdlog.h>

using namespace embview::core;

void SerialTransport::PortDeleter::operator()(sp_port* port) const
{
    if (port)
    {
        sp_close(port);
        sp_free_port(port);
    }
}

SerialTransport::SerialTransport(SerialConfig config)
    : m_config(std::move(config))
{
}

SerialTransport::~SerialTransport()
{
    if (m_isOpen)
    {
        close();
    }
}

SerialTransport::SerialTransport(SerialTransport&&) noexcept = default;
SerialTransport& SerialTransport::operator=(SerialTransport&&) noexcept = default;

bool SerialTransport::open()
{
    if (m_isOpen)
    {
        return true;
    }

    sp_port* rawPort = nullptr;
    if (sp_get_port_by_name(m_config.portName.c_str(), &rawPort) != SP_OK)
    {
        spdlog::error("Failed to find serial port: {}", m_config.portName);
        return false;
    }

    m_port.reset(rawPort);

    if (sp_open(m_port.get(), SP_MODE_READ_WRITE) != SP_OK)
    {
        spdlog::error("Failed to open serial port: {}", m_config.portName);
        m_port.reset();
        return false;
    }

    sp_set_baudrate(m_port.get(), m_config.baudRate);
    sp_set_bits(m_port.get(), m_config.dataBits);
    sp_set_stopbits(m_port.get(), m_config.stopBits);
    sp_set_parity(m_port.get(), static_cast<sp_parity>(m_config.parity));
    sp_set_flowcontrol(m_port.get(), SP_FLOWCONTROL_NONE);

    m_isOpen = true;
    spdlog::info("Opened serial port: {} at {} baud", m_config.portName, m_config.baudRate);
    return true;
}

void SerialTransport::close()
{
    if (!m_isOpen)
    {
        return;
    }

    m_isOpen = false;
    m_port.reset();
    spdlog::info("Closed serial port: {}", m_config.portName);
}

bool SerialTransport::isOpen() const
{
    return m_isOpen;
}

std::vector<uint8_t> SerialTransport::read(std::size_t maxBytes)
{
    if (!m_isOpen || !m_port)
    {
        return {};
    }

    std::vector<uint8_t> buffer(maxBytes);
    int bytesRead = sp_nonblocking_read(m_port.get(), buffer.data(), static_cast<int>(maxBytes));

    if (bytesRead < 0)
    {
        spdlog::warn("Serial read error on port: {}", m_config.portName);
        return {};
    }

    buffer.resize(static_cast<std::size_t>(bytesRead));
    return buffer;
}

std::size_t SerialTransport::write(std::span<const uint8_t> data)
{
    if (!m_isOpen || !m_port)
    {
        return 0;
    }

    int bytesWritten = sp_nonblocking_write(m_port.get(), data.data(), static_cast<int>(data.size()));

    if (bytesWritten < 0)
    {
        spdlog::warn("Serial write error on port: {}", m_config.portName);
        return 0;
    }

    return static_cast<std::size_t>(bytesWritten);
}

std::vector<std::string> SerialTransport::listPorts()
{
    std::vector<std::string> result;

    sp_port** rawList = nullptr;
    if (sp_list_ports(&rawList) != SP_OK)
    {
        return result;
    }

    auto listGuard = std::unique_ptr<sp_port*, decltype([](sp_port** p) { sp_free_port_list(p); })>(rawList);

    for (int i = 0; rawList[i] != nullptr; ++i)
    {
        result.emplace_back(sp_get_port_name(rawList[i]));
    }

    return result;
}
