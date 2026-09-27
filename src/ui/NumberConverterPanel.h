#pragma once

/**
 * @namespace embview::ui Ui support.
 */
namespace embview::ui
{
    /**
     * @brief Panel for converting numbers between decimal, hex, binary, and octal.
     *
     * Supports 8/16/32/64-bit integers (signed/unsigned) and IEEE 754 float/double
     * representation display.
     */
    class NumberConverterPanel
    {
    public:
        NumberConverterPanel() = default;
        ~NumberConverterPanel() = default;

        /**
         * @brief Render the numeric conversion UI.
         * @param open Whether the panel is open.
         */
        void render(bool& open);

    private:
        char m_input[128] = {};
        int m_bitWidth = 2;   // 0=8, 1=16, 2=32, 3=64
        bool m_isSigned = true;
    };
} // namespace embview::ui
