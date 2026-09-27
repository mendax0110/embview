#pragma once

#include <memory>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Trigger Engine.
     * \class TriggerEngine
     */
    class TriggerEngine;

    /**
     * @brief Data Store.
     * \class DataStore
     */
    class DataStore;
} // namespace embview::core

/**
 * @namespace embview::ui Ui support.
 */
namespace embview::ui
{
    /**
     * @brief Panel for configuring per-channel triggers/alarms.
     */
    class TriggerPanel
    {
    public:
        /**
         * @brief Create a trigger configuration panel.
         * @param engine Trigger engine to configure.
         * @param dataStore Shared data store for previewing channel values.
         */
        TriggerPanel(std::shared_ptr<core::TriggerEngine> engine,
                    std::shared_ptr<core::DataStore> dataStore);
        /**
         * @brief Destroy the trigger panel.
         */
        ~TriggerPanel();

        /**
         * @brief Render the trigger UI.
         * @param open Whether the panel is open.
         */
        void render(bool& open) const;

    private:
        std::shared_ptr<core::TriggerEngine> m_engine;
        std::shared_ptr<core::DataStore> m_dataStore;
    };
} // namespace embview::ui
