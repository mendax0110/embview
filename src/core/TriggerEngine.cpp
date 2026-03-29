#include "core/TriggerEngine.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <format>

using namespace embview::core;

void TriggerEngine::setCallback(Callback cb)
{
    std::lock_guard lock(m_mutex);
    m_callback = std::move(cb);
}

std::size_t TriggerEngine::addTrigger(TriggerConfig config)
{
    std::lock_guard lock(m_mutex);
    m_triggers.push_back(std::move(config));
    return m_triggers.size() - 1;
}

void TriggerEngine::removeTrigger(const std::size_t index)
{
    std::lock_guard lock(m_mutex);
    if (index < m_triggers.size())
    {
        m_triggers.erase(m_triggers.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

void TriggerEngine::evaluate(const DataFrame& frame)
{
    std::lock_guard lock(m_mutex);

    for (std::size_t i = 0; i < m_triggers.size(); ++i)
    {
        auto&[channel, condition, threshold, enabled, label] = m_triggers[i];
        if (!enabled || channel != frame.channel)
        {
            continue;
        }

        bool fired = false;
        double prevVal = 0.0;
        const bool hasPrev = m_lastValues.contains(i);
        if (hasPrev)
        {
            prevVal = m_lastValues[i];
        }

        switch (condition)
        {
            case TriggerCondition::Above:
                fired = frame.value > threshold;
                break;
            case TriggerCondition::Below:
                fired = frame.value < threshold;
                break;
            case TriggerCondition::Equal:
                fired = std::abs(frame.value - threshold) < 1e-9;
                break;
            case TriggerCondition::RisingEdge:
                fired = hasPrev && prevVal <= threshold && frame.value > threshold;
                break;
            case TriggerCondition::FallingEdge:
                fired = hasPrev && prevVal >= threshold && frame.value < threshold;
                break;
        }

        m_lastValues[i] = frame.value;

        if (fired)
        {
            TriggerEvent event;
            event.triggerIndex = i;
            event.frame = frame;
            event.message = std::format("[Trigger] {} ch={} val={:.4f} threshold={:.4f}",
                                         label.empty() ? std::format("#{}", i) : label,
                                         frame.channel, frame.value, threshold);

            spdlog::warn("{}", event.message);
            m_events.push_back(event);

            // Cap events
            if (m_events.size() > 1000)
            {
                m_events.erase(m_events.begin());
            }

            if (m_callback)
            {
                m_callback(event);
            }
        }
    }
}

std::vector<TriggerConfig>& TriggerEngine::triggers()
{
    return m_triggers;
}

std::vector<TriggerEvent> TriggerEngine::recentEvents(const std::size_t maxCount) const
{
    std::lock_guard lock(m_mutex);
    if (m_events.size() <= maxCount)
    {
        return m_events;
    }
    return {m_events.end() - static_cast<std::ptrdiff_t>(maxCount), m_events.end()};
}

void TriggerEngine::clearEvents()
{
    std::lock_guard lock(m_mutex);
    m_events.clear();
}
