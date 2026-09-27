#pragma once

#include <memory>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
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
     * @brief Panel showing per-channel statistics (min, max, mean, stddev).
     */
    class StatsPanel
    {
    public:
        /**
         * @brief Create a panel for per-channel summary statistics.
         * @param dataStore Shared store containing channel samples.
         */
        explicit StatsPanel(std::shared_ptr<core::DataStore> dataStore);
        ~StatsPanel();

        /**
         * @brief Render the statistics dashboard.
         * @param open Whether the panel is open.
         */
        void render(bool& open) const;

    private:
        std::shared_ptr<core::DataStore> m_dataStore;
    };
} // namespace embview::ui
