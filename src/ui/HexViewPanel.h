#pragma once

#include <memory>

namespace embview::core
{
    class RawDataBuffer;
} // namespace embview::core

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
        explicit HexViewPanel(std::shared_ptr<core::RawDataBuffer> rawBuffer);
        ~HexViewPanel();

        void render(bool& open);

    private:
        std::shared_ptr<core::RawDataBuffer> m_rawBuffer;
        bool m_autoScroll = true;
        bool m_paused = false;
    };
} // namespace embview::ui
