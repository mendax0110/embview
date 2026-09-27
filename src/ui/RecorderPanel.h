#pragma once

#include <memory>
#include <string>
#include <vector>

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

    /**
     * @brief Session Recorder.
     * \class SessionRecorder
     */
    class SessionRecorder;
} // namespace embview::core

/**
 * @namespace embview::ui Ui support.
 */
namespace embview::ui
{
    /**
     * @brief Panel for recording sessions and replaying saved data.
     */
    class RecorderPanel
    {
    public:
        /**
         * @brief Create a panel for recording and replaying sessions.
         * @param dataStore Shared data store to record.
         * @param recorder Session recorder used for disk I/O.
         */
        RecorderPanel(std::shared_ptr<core::DataStore> dataStore,
                      std::shared_ptr<core::SessionRecorder> recorder);
        /**
         * @brief Destroy the recorder panel.
         */
        ~RecorderPanel();

        /**
         * @brief Render the recording UI.
         * @param open Whether the panel is open.
         */
        void render(bool& open);

    private:
        std::shared_ptr<core::DataStore> m_dataStore;
        std::shared_ptr<core::SessionRecorder> m_recorder;
        char m_filename[256] = "session.emb";
        std::vector<std::string> m_recentFiles;
    };
} // namespace embview::ui
