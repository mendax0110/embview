#include "core/RawDataBuffer.h"

#include <algorithm>

using namespace embview::core;

RawDataBuffer::RawDataBuffer(std::size_t capacity)
    : m_capacity(capacity)
{
    m_buffer.reserve(capacity);
}

void RawDataBuffer::push(const std::vector<uint8_t>& data)
{
    std::lock_guard lock(m_mutex);

    m_buffer.insert(m_buffer.end(), data.begin(), data.end());

    if (m_buffer.size() > m_capacity)
    {
        auto excess = m_buffer.size() - m_capacity;
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + static_cast<std::ptrdiff_t>(excess));
    }
}

std::vector<uint8_t> RawDataBuffer::snapshot() const
{
    std::lock_guard lock(m_mutex);
    return m_buffer;
}

void RawDataBuffer::clear()
{
    std::lock_guard lock(m_mutex);
    m_buffer.clear();
}

std::size_t RawDataBuffer::size() const
{
    std::lock_guard lock(m_mutex);
    return m_buffer.size();
}
