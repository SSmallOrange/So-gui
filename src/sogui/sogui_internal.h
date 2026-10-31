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

// Context
struct Context {
    unsigned long long frameCount = 0;
    std::atomic<bool>  redrawRequested{false};
    ThemeState         theme;
};

} // namespace SoGui
