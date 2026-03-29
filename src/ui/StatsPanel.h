#pragma once

#include <memory>

namespace embview::core
{
    class DataStore;
} // namespace embview::core

namespace embview::ui
{
    /**
     * @brief Panel showing per-channel statistics (min, max, mean, stddev).
     */
    class StatsPanel
    {
    public:
        explicit StatsPanel(std::shared_ptr<core::DataStore> dataStore);
        ~StatsPanel();

        void render(bool& open) const;

    private:
        std::shared_ptr<core::DataStore> m_dataStore;
    };
} // namespace embview::ui
