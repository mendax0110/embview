#pragma once

#include <memory>

namespace embview::core
{
    class TriggerEngine;
    class DataStore;
} // namespace embview::core

namespace embview::ui
{
    /**
     * @brief Panel for configuring per-channel triggers/alarms.
     */
    class TriggerPanel
    {
    public:
        TriggerPanel(std::shared_ptr<core::TriggerEngine> engine,
                     std::shared_ptr<core::DataStore> dataStore);
        ~TriggerPanel();

        void render(bool& open);

    private:
        std::shared_ptr<core::TriggerEngine> m_engine;
        std::shared_ptr<core::DataStore> m_dataStore;
    };
} // namespace embview::ui
