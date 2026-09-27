#pragma once

#include <mutex>
#include <string>
#include <vector>

#include <spdlog/sinks/base_sink.h>

/**
 * @namespace embview::ui Ui support.
 */
namespace embview::ui
{
    /// @brief Entry in the log panel display buffer. \struct LogEntry
    struct LogEntry
    {
        spdlog::level::level_enum level;
        std::string message;
    };

    /**
    * @brief Log output panel with spdlog sink integration.
    *
    * Captures spdlog messages via a custom sink and displays them
    * in a scrollable, color-coded view.
    */
    class LogPanel
    {
    public:
        /**
         * @brief Construct the log panel.
         */
        LogPanel();

        /**
         * @brief Destroy the log panel.
         */
        ~LogPanel();

        /**
         * @brief Render the log panel.
         * @param open Visibility toggle controlled by the parent window.
         */
        void render(bool& open);

        /**
         * @brief Get the spdlog sink for registration.
         * @return Shared pointer to the custom ImGui sink.
         */
        [[nodiscard]] std::shared_ptr<spdlog::sinks::sink> getSink() const;

    private:
        /**
         * @brief Im Gui Sink.
         * \class ImGuiSink
         */
        class ImGuiSink;
        std::shared_ptr<ImGuiSink> m_sink;
        bool m_autoScroll = true;
    };
} // namespace embview::ui
