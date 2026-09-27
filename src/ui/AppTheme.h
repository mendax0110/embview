#pragma once

#include <imgui.h>

/**
 * @brief Color Mode.
 * \enum ColorMode
 */
enum class ColorMode
{
    Light,
    Dark
};

/**
 * @namespace embview::ui::appTheme App Theme support.
 */
namespace embview::ui::appTheme
{
    /**
     * @brief Apply the selected theme and DPI scale.
     * @param mode UI color mode.
     * @param dpiScale DPI scale factor.
     */
    void apply(ColorMode mode, float dpiScale = 1.0f);
}