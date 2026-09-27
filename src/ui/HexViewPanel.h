#pragma once

#include <memory>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Raw Data Buffer.
     * \class RawDataBuffer
     */
    class RawDataBuffer;
} // namespace embview::core

/**
 * @namespace embview::ui Ui support.
 */
namespace embview::ui
{
    /**
     * @brief Panel displaying raw incoming bytes as a hex dump.
     *
     * Shows address offsets, hex bytes, and ASCII representation
     * side by side. Auto-scrolls to latest data.
     */
    class HexViewPanel
    {
    public:
        /**
         * @brief Create a panel for displaying raw incoming bytes.
         * @param rawBuffer Buffer containing captured raw bytes.
         */
        explicit HexViewPanel(std::shared_ptr<core::RawDataBuffer> rawBuffer);
        ~HexViewPanel();

        /**
         * @brief Render the raw hex dump UI.
         * @param open Whether the panel is open.
         */
        void render(bool& open);

    private:
        std::shared_ptr<core::RawDataBuffer> m_rawBuffer;
        bool m_autoScroll = true;
        bool m_paused = false;
    };
} // namespace embview::ui
