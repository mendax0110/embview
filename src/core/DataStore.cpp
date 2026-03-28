#include "core/DataStore.h"
#include "core/DiagnosticRegistry.h"
#include <mutex>

using namespace embview::core;

DataStore::DataStore(std::size_t maxSamplesPerChannel)
    : m_maxSamples(maxSamplesPerChannel)
{
    DIAG_REGISTER_MUTEX("DataStore::m_mutex", &m_mutex);
}

DataStore::~DataStore()
{
    DIAG_UNREGISTER_MUTEX("DataStore::m_mutex");
}

void DataStore::push(const DataFrame& frame)
{
    std::unique_lock lock(m_mutex);
    auto& deque = m_channels[frame.channel];
    deque.push_back(frame);

    while (deque.size() > m_maxSamples)
    {
        deque.pop_front();
    }
}

std::vector<DataFrame> DataStore::getChannel(uint16_t channel) const
{
    std::shared_lock lock(m_mutex);
    auto it = m_channels.find(channel);
    if (it == m_channels.end())
    {
        return {};
    }
    return {it->second.begin(), it->second.end()};
}

std::vector<uint16_t> DataStore::getActiveChannels() const
{
    std::shared_lock lock(m_mutex);
    std::vector<uint16_t> result;
    result.reserve(m_channels.size());
    for (const auto& [channel, _] : m_channels)
    {
        result.push_back(channel);
    }
    return result;
}

std::size_t DataStore::getChannelSize(uint16_t channel) const
{
    std::shared_lock lock(m_mutex);
    auto it = m_channels.find(channel);
    if (it == m_channels.end())
    {
        return 0;
    }
    return it->second.size();
}

void DataStore::clear()
{
    std::unique_lock lock(m_mutex);
    m_channels.clear();
}

void DataStore::clearChannel(uint16_t channel)
{
    std::unique_lock lock(m_mutex);
    m_channels.erase(channel);
}
