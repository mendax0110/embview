#pragma once

#include "core/Protocol.h"

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Trigger Condition.
     * \enum TriggerCondition
     */
    enum class TriggerCondition
    {
        Above,
        Below,
        Equal,
        RisingEdge,
        FallingEdge
    };

    /**
     * @brief Trigger Config.
     * \struct TriggerConfig
     */
    struct TriggerConfig
    {
        uint16_t channel = 0;
        TriggerCondition condition = TriggerCondition::Above;
        double threshold = 0.0;
        bool enabled = true;
        std::string label;
    };

    /**
     * @brief Trigger Event.
     * \struct TriggerEvent
     */
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
        /**
         * @brief Trigger callback signature.
         */
        using Callback = std::function<void(const TriggerEvent&)>;

        /**
         * @brief Set the trigger callback.
         * @param cb Callback to invoke when a trigger fires.
         */
        void setCallback(Callback cb);

        /**
         * @brief Add a trigger rule.
         * @param config Trigger configuration.
         * @return The trigger index.
         */
        std::size_t addTrigger(TriggerConfig config);

        /**
         * @brief Remove a trigger rule.
         * @param index Trigger index to remove.
         */
        void removeTrigger(std::size_t index);

        /**
         * @brief Evaluate a frame against the configured triggers.
         * @param frame Incoming frame to test.
         */
        void evaluate(const DataFrame& frame);

        /**
         * @brief Get the trigger configuration list.
         * @return Mutable trigger definitions.
         */
        std::vector<TriggerConfig>& triggers();

        /**
         * @brief Get the most recent trigger events.
         * @param maxCount Maximum number of events to return.
         * @return Recent trigger events.
         */
        std::vector<TriggerEvent> recentEvents(std::size_t maxCount = 100) const;

        /**
         * @brief Clear all recorded trigger events.
         */
        void clearEvents();

    private:
        mutable std::mutex m_mutex;
        std::vector<TriggerConfig> m_triggers;
        std::vector<TriggerEvent> m_events;
        std::unordered_map<std::size_t, double> m_lastValues;
        Callback m_callback;
    };
} // namespace embview::core
