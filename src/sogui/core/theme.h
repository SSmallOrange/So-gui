#pragma once

#include <imgui.h>

namespace SoGui {

enum class ThemeId { Light, Dark };

enum class Hue { Gray, GrayAlpha, Blue, Red, Amber, Green, Teal, Purple, Pink, COUNT };

enum class Col {
    Bg1,           // 默认底色
    Bg2,
    Fill,          // 组件背景
    FillHover,
    FillActive,
    Border,        // 边框
    BorderHover,
    BorderActive,
    BorderAlpha,
    TextPrimary,
    TextSecondary,
    TextDisabled,
    Accent,
    AccentHover,
    FocusRing,
    Selection,
    Backdrop,
    COUNT
};

// 主题切换下一帧生效
void    SetTheme(ThemeId theme);
// 最近一次 SetTheme 的目标主题
ThemeId GetTheme();

ImVec4      GetColor(Col col);
ImU32       GetColorU32(Col col, float alphaMul = 1.0f);
const char* GetColorName(Col col);

ImVec4 GetPaletteColor(Hue hue, int level);
ImU32  GetPaletteColorU32(Hue hue, int level, float alphaMul = 1.0f);

// 只对 SoGui:: 组件生效
void PushColor(Col col, const ImVec4& color);
void PopColor(int count = 1);

// 把当前主题写入 ImGuiStyle.Color
void ApplyToImGuiStyle(ImGuiStyle& style);

} // namespace SoGui
