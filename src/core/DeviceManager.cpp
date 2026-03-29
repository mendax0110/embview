#include "core/DeviceManager.h"

#include "core/DataStore.h"
#include "core/DiagnosticRegistry.h"
#include "core/RawDataBuffer.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>

using namespace embview::core;

DeviceManager::DeviceManager(std::shared_ptr<DataStore> dataStore)
    : m_dataStore(std::move(dataStore))
    , m_rawDataBuffer(std::make_shared<RawDataBuffer>())
{
    DIAG_REGISTER_MUTEX("DeviceManager::m_mutex", &m_mutex);
}

DeviceManager::~DeviceManager()
{
    removeAll();
    DIAG_UNREGISTER_MUTEX("DeviceManager::m_mutex");
}

void DeviceManager::addDevice(std::string name, std::shared_ptr<ITransport> transport, const ProtocolMode protocolMode)
{
    std::unique_lock lock(m_mutex);

    for (const auto& session : m_sessions)
    {
        if (session->name == name)
        {
            spdlog::warn("Device '{}' already exists", name);
            return;
        }
    }

    auto session = std::make_unique<DeviceSession>();
    session->name = std::move(name);
    session->deviceIndex = nextDeviceIndex();
    session->transport = std::move(transport);

    switch (protocolMode)
    {
        case ProtocolMode::AsciiLine:
            session->parser = std::make_unique<AsciiLineParser>();
            break;
        case ProtocolMode::AsciiCsv:
            session->parser = std::make_unique<AsciiCsvParser>();
            break;
        case ProtocolMode::Binary:
        default:
            session->parser = std::make_unique<BinaryProtocolParser>();
            break;
    }

    auto& ref = *session;
    session->readerThread = std::jthread([this, &ref](const std::stop_token& st)
    {
        readerLoop(ref, st);
    });

    spdlog::info("Device '{}' added (index {})", ref.name, ref.deviceIndex);
    m_sessions.push_back(std::move(session));
}

void DeviceManager::removeDevice(const std::string& name)
{
    std::unique_lock lock(m_mutex);

    const auto it = std::ranges::find_if(m_sessions, [&name](const auto& s)
    {
        return s->name == name;
    });

    if (it == m_sessions.end())
    {
        return;
    }

    (*it)->readerThread.request_stop();
    auto session = std::move(*it);
    m_sessions.erase(it);

    lock.unlock();

    session.reset();

    spdlog::info("Device '{}' removed", name);
}

void DeviceManager::removeAll()
{
    std::unique_lock lock(m_mutex);

    for (const auto& session : m_sessions)
    {
        session->readerThread.request_stop();
    }

    auto sessions = std::move(m_sessions);
    m_sessions.clear();
    lock.unlock();

    sessions.clear();
}

std::vector<std::string> DeviceManager::getDeviceNames() const
{
    std::shared_lock lock(m_mutex);
    std::vector<std::string> names;
    names.reserve(m_sessions.size());
    for (const auto& session : m_sessions)
    {
        names.push_back(session->name);
    }
    return names;
}

bool DeviceManager::isDeviceConnected(const std::string& name) const
{
    std::shared_lock lock(m_mutex);
    for (const auto& session : m_sessions)
    {
        if (session->name == name)
        {
            return session->transport && session->transport->isOpen();
        }
    }
    return false;
}

std::size_t DeviceManager::deviceCount() const
{
    std::shared_lock lock(m_mutex);
    return m_sessions.size();
}

void DeviceManager::sendCommand(const std::string& deviceName, const DataFrame& frame) const
{
    std::shared_lock lock(m_mutex);
    for (const auto& session : m_sessions)
    {
        if (session->name == deviceName && session->transport && session->transport->isOpen())
        {
            try
            {
                auto encoded = Protocol::encode(frame);
                session->transport->write(encoded);
                spdlog::info("Sent command to '{}': ch={} val={}", deviceName, frame.channel, frame.value);
            }
            catch (const std::exception& e)
            {
                spdlog::error("Failed to send command to '{}': {}", deviceName, e.what());
            }
            return;
        }
    }
    spdlog::warn("Cannot send to '{}': device not found or not connected", deviceName);
}

void DeviceManager::sendRaw(const std::string& deviceName, const std::span<const uint8_t> data) const
{
    std::shared_lock lock(m_mutex);
    for (const auto& session : m_sessions)
    {
        if (session->name == deviceName && session->transport && session->transport->isOpen())
        {
            try
            {
                session->transport->write(data);
                spdlog::info("Sent {} raw bytes to '{}'", data.size(), deviceName);
            }
            catch (const std::exception& e)
            {
                spdlog::error("Failed to send raw data to '{}': {}", deviceName, e.what());
            }
            return;
        }
    }
    spdlog::warn("Cannot send raw to '{}': device not found or not connected", deviceName);
}

std::shared_ptr<RawDataBuffer> DeviceManager::getRawDataBuffer() const
{
    return m_rawDataBuffer;
}

void DeviceManager::readerLoop(DeviceSession& session, const std::stop_token& stopToken) const
{
    spdlog::info("Reader thread started for '{}'", session.name);

    while (!stopToken.stop_requested())
    {
        try
        {
            if (!session.transport || !session.transport->isOpen())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            auto bytes = session.transport->read(4096);
            if (!bytes.empty())
            {
                m_rawDataBuffer->push(bytes);

                session.parser->feedData(bytes);
                while (auto frame = session.parser->parseNext())
                {
                    DataFrame remapped = *frame;
                    remapped.channel = static_cast<uint16_t>(session.deviceIndex) * 256 + frame->channel;
                    m_dataStore->push(remapped);
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        catch (const std::exception& e)
        {
            spdlog::error("Reader error for '{}': {}", session.name, e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    spdlog::info("Reader thread stopped for '{}'", session.name);
}

uint8_t DeviceManager::nextDeviceIndex()
{
    return m_nextIndex++;
}
