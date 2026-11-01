#pragma once

// 扩展组件用的内部接口，不保证 ABI 稳定。

#include <sogui/sogui.h>

#include <imgui.h>

#include <atomic>

namespace SoGui {
// Theme
inline constexpr int kColCount = static_cast<int>(Col::COUNT);

struct ColorMod {
    Col    col;
    ImVec4 backup;
};

struct ThemeState {
    ThemeId            active = ThemeId::Light;
    ThemeId            requested = ThemeId::Light;
    bool               applyToImGuiStyle = true;
    bool               styleDirty = false;          // 主题已切换，等待写入 ImGuiStyle
    ImVec4             colors[kColCount] = {};      // 当前Color
    ImU32              colorsU32[kColCount] = {};
    ImVector<ColorMod> colorStack;
};

void ThemeInitialize(ThemeState& state, const ContextConfig& config);
void ThemeNewFrame(ThemeState& state);
void ThemeEndFrame(const ThemeState& state);
void ApplyToImGuiStyle(const ThemeState& state, ImGuiStyle& style);

// Font
inline constexpr int kFontWeightCount = static_cast<int>(FontWeight::COUNT);

struct FontEntry {
    ImFont* font = nullptr;        // 属于 ImGui 图集
    float   heightPerEm = 1.0f;    // (ascender - descender) / unitsPerEm
    float   ascentRatio = 0.0f;    // ascender / (ascender - descender)
};

struct FontState {
    FontEntry entries[kFontWeightCount];
};

// 字体不在当前 ImGui 图集中时返回 nullptr
const FontEntry* FindFont(FontWeight weight);

// Context
struct Context {
    unsigned long long frameCount = 0;
    std::atomic<bool>  redrawRequested{false};
    ThemeState         theme;
    FontState          fonts;
};

} // namespace SoGui
