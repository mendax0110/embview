#pragma once

#include "core/Protocol.h"

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace embview::core
{
    enum class TriggerCondition
    {
        Above,
        Below,
        Equal,
        RisingEdge,
        FallingEdge
    };

    struct TriggerConfig
    {
        uint16_t channel = 0;
        TriggerCondition condition = TriggerCondition::Above;
        double threshold = 0.0;
        bool enabled = true;
        std::string label;
    };

    struct TriggerEvent
    {
        std::size_t triggerIndex;
        DataFrame frame;
        std::string message;
    };

    /**
     * @brief Evaluates incoming data frames against user-defined triggers.
     *
     * When a trigger fires, the event is stored and a callback is invoked.
     */
    class TriggerEngine
    {
    public:
        using Callback = std::function<void(const TriggerEvent&)>;

        void setCallback(Callback cb);

        std::size_t addTrigger(TriggerConfig config);

        void removeTrigger(std::size_t index);

        void evaluate(const DataFrame& frame);

        std::vector<TriggerConfig>& triggers();

        std::vector<TriggerEvent> recentEvents(std::size_t maxCount = 100) const;

        void clearEvents();

    private:
        mutable std::mutex m_mutex;
        std::vector<TriggerConfig> m_triggers;
        std::vector<TriggerEvent> m_events;
        std::unordered_map<std::size_t, double> m_lastValues;
        Callback m_callback;
    };
} // namespace embview::core
