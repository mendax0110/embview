#pragma once

#include <memory>
#include <string>
#include <vector>

namespace embview::core
{
    class DataStore;
    class SessionRecorder;
} // namespace embview::core

namespace embview::ui
{
    /**
     * @brief Panel for recording sessions and replaying saved data.
     */
    class RecorderPanel
    {
    public:
        RecorderPanel(std::shared_ptr<core::DataStore> dataStore,
                      std::shared_ptr<core::SessionRecorder> recorder);
        ~RecorderPanel();

        void render(bool& open);

    private:
        std::shared_ptr<core::DataStore> m_dataStore;
        std::shared_ptr<core::SessionRecorder> m_recorder;
        char m_filename[256] = "session.emb";
        std::vector<std::string> m_recentFiles;
    };
} // namespace embview::ui
