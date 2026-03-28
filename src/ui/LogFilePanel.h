#pragma once

#include <memory>

namespace embview::core
{
    class LogFileManager;
} // namespace embview::core

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
        explicit LogFilePanel(std::shared_ptr<core::LogFileManager> logMgr);
        ~LogFilePanel();

        void render(bool& open);

    private:
        std::shared_ptr<core::LogFileManager> m_logMgr;
        int m_deleteAgeDays = 30;
    };
} // namespace embview::ui
