#include "AppTheme.h"
#include <memory>

using namespace embview::ui::appTheme;

static ImVec4 hex(const unsigned int rgb, const float a = 1.0f)
{
    return ImVec4(
        ((rgb >> 16) & 0xFF) / 255.0f,
        ((rgb >> 8) & 0xFF) / 255.0f,
        ((rgb >> 0) & 0xFF) / 255.0f,
        a
    );
}

static ImVec4 lerp(const ImVec4 a, const ImVec4 b, const float t)
{
    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    );
}

struct Palette
{
    ImVec4 bg, surface, surface2, border, borderHov;
    ImVec4 textPri, textSec, textHint;
    ImVec4 accent, accentHov, accentAct, accentText;
    ImVec4 ok, okBg, warn, warnBg, err, errBg;
    ImVec4 transparent;
};

static Palette lightPalette()
{
    Palette p{};
    p.bg = hex(0xF7F8FA);
    p.surface = hex(0xEEF1F7);
    p.surface2 = hex(0xE5E9F2);
    p.border = hex(0xDDE1E9);
    p.borderHov = hex(0xC5CCDB);
    p.textPri = hex(0x1A1D23);
    p.textSec = hex(0x555B6A);
    p.textHint = hex(0x9199AC);
    p.accent = hex(0x4A6FA5);
    p.accentHov = hex(0x3A5A8C);
    p.accentAct = hex(0x2F4D7E);
    p.accentText = hex(0xFFFFFF);
    p.ok = hex(0x1B7A4A);
    p.okBg = hex(0xDFF2EA);
    p.warn = hex(0x8A5F00);
    p.warnBg = hex(0xFFF3DC);
    p.err = hex(0xC0392B);
    p.errBg = hex(0xFDEAEA);
    p.transparent = ImVec4(0,0,0,0);
    return p;
}

static Palette darkPalette()
{
    Palette p{};
    p.bg = hex(0x16181E);
    p.surface = hex(0x1E2130);
    p.surface2 = hex(0x252832);
    p.border = hex(0x2C2F3A);
    p.borderHov = hex(0x3A3F50);
    p.textPri = hex(0xE2E5EE);
    p.textSec = hex(0x8A90A3);
    p.textHint = hex(0x545B71);
    p.accent = hex(0x5B83C3);
    p.accentHov = hex(0x6E96D6);
    p.accentAct = hex(0x7EA3D8);
    p.accentText = hex(0xFFFFFF);
    p.ok = hex(0x4EC98A);
    p.okBg = hex(0x1A3D2E);
    p.warn = hex(0xF5B942);
    p.warnBg = hex(0x3D2E0A);
    p.err = hex(0xF07070);
    p.errBg = hex(0x3D0F0F);
    p.transparent = ImVec4(0,0,0,0);
    return p;
}

void embview::ui::appTheme::apply(const ColorMode mode, float dpiScale)
{
    if (dpiScale < 0.5f)
    {
        dpiScale = 1.0f;
    }

    const Palette& p = (mode == ColorMode::Dark) ? darkPalette() : lightPalette();

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* c = style.Colors;

    c[ImGuiCol_Text] = p.textPri;
    c[ImGuiCol_TextDisabled] = p.textHint;

    c[ImGuiCol_WindowBg] = p.bg;
    c[ImGuiCol_ChildBg] = p.bg;
    c[ImGuiCol_PopupBg] = p.surface;

    c[ImGuiCol_Border] = p.border;
    c[ImGuiCol_BorderShadow] = p.transparent;

    c[ImGuiCol_FrameBg] = p.surface;
    c[ImGuiCol_FrameBgHovered] = p.surface2;
    c[ImGuiCol_FrameBgActive] = p.surface2;

    c[ImGuiCol_TitleBg] = p.bg;
    c[ImGuiCol_TitleBgActive] = p.surface;
    c[ImGuiCol_TitleBgCollapsed] = p.bg;

    c[ImGuiCol_MenuBarBg] = p.surface;

    c[ImGuiCol_ScrollbarBg] = p.bg;
    c[ImGuiCol_ScrollbarGrab] = p.border;
    c[ImGuiCol_ScrollbarGrabHovered] = p.borderHov;
    c[ImGuiCol_ScrollbarGrabActive] = p.textHint;

    c[ImGuiCol_CheckMark] = p.accent;
    c[ImGuiCol_SliderGrab] = p.accent;
    c[ImGuiCol_SliderGrabActive] = p.accentHov;

    c[ImGuiCol_Button] = p.surface;
    c[ImGuiCol_ButtonHovered] = p.surface2;
    c[ImGuiCol_ButtonActive] = p.accent;

    c[ImGuiCol_Header] = p.surface;
    c[ImGuiCol_HeaderHovered] = p.surface2;
    c[ImGuiCol_HeaderActive] = p.accent;

    c[ImGuiCol_Separator] = p.border;
    c[ImGuiCol_SeparatorHovered] = p.accentHov;
    c[ImGuiCol_SeparatorActive] = p.accent;

    c[ImGuiCol_ResizeGrip] = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.18f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(p.accent.x, p.accent.y, p.accent.x, 0.55f);
    c[ImGuiCol_ResizeGripActive] = p.accent;

    c[ImGuiCol_Tab] = p.bg;
    c[ImGuiCol_TabHovered] = p.surface2;
    c[ImGuiCol_TabSelected] = p.surface;
    c[ImGuiCol_TabDimmed] = p.bg;
    c[ImGuiCol_TabDimmedSelected] = p.surface;

    c[ImGuiCol_DockingPreview] = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.45f);
    c[ImGuiCol_DockingEmptyBg] = p.bg;

    c[ImGuiCol_TextSelectedBg] = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.70f);
    c[ImGuiCol_NavHighlight] = p.accent;
    c[ImGuiCol_NavWindowingHighlight] = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.70f);
    c[ImGuiCol_NavWindowingDimBg] = ImVec4(0, 0, 0, 0.20f);
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.40f);

    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;

    style.WindowPadding = ImVec2(12.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 18.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;

    style.ScaleAllSizes(dpiScale);
}