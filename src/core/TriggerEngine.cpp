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

void TriggerEngine::removeTrigger(std::size_t index)
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
        auto& trig = m_triggers[i];
        if (!trig.enabled || trig.channel != frame.channel)
        {
            continue;
        }

        bool fired = false;
        double prevVal = 0.0;
        bool hasPrev = m_lastValues.contains(i);
        if (hasPrev)
        {
            prevVal = m_lastValues[i];
        }

        switch (trig.condition)
        {
            case TriggerCondition::Above:
                fired = frame.value > trig.threshold;
                break;
            case TriggerCondition::Below:
                fired = frame.value < trig.threshold;
                break;
            case TriggerCondition::Equal:
                fired = std::abs(frame.value - trig.threshold) < 1e-9;
                break;
            case TriggerCondition::RisingEdge:
                fired = hasPrev && prevVal <= trig.threshold && frame.value > trig.threshold;
                break;
            case TriggerCondition::FallingEdge:
                fired = hasPrev && prevVal >= trig.threshold && frame.value < trig.threshold;
                break;
        }

        m_lastValues[i] = frame.value;

        if (fired)
        {
            TriggerEvent event;
            event.triggerIndex = i;
            event.frame = frame;
            event.message = std::format("[Trigger] {} ch={} val={:.4f} threshold={:.4f}",
                                         trig.label.empty() ? std::format("#{}", i) : trig.label,
                                         frame.channel, frame.value, trig.threshold);

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

std::vector<TriggerEvent> TriggerEngine::recentEvents(std::size_t maxCount) const
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
