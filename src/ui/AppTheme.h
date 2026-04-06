#pragma once

#include <imgui.h>

enum class ColorMode
{
    Light,
    Dark
};

namespace embview::ui::appTheme
{
    void apply(ColorMode mode, float dpiScale = 1.0f);
}