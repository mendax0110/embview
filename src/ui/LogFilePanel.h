#pragma once

#include <memory>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Log File Manager.
     * \class LogFileManager
     */
    class LogFileManager;
} // namespace embview::core

/**
 * @namespace embview::ui Ui support.
 */
namespace embview::ui
{
    /**
     * @brief Panel for viewing and managing log files.
     *
     * Lists log files on disk with size and date, and provides
     * controls to delete individual files or purge old ones.
     */
    class LogFilePanel
    {
    public:
        /**
         * @brief Create a panel for browsing log files.
         * @param logMgr Shared log manager used to enumerate and delete files.
         */
        explicit LogFilePanel(std::shared_ptr<core::LogFileManager> logMgr);
        ~LogFilePanel();

        /**
         * @brief Render the log-file management UI.
         * @param open Whether the panel is open.
         */
        void render(bool& open);

    private:
        std::shared_ptr<core::LogFileManager> m_logMgr;
        int m_deleteAgeDays = 30;
    };
} // namespace embview::ui
