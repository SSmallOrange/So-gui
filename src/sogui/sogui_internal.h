#pragma once

// 扩展组件用的内部接口，不保证 ABI 稳定。

#include <sogui/sogui.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <atomic>
#include <string_view>

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

// 当前帧生效的主题；SetTheme 在下一帧才会改变它
ThemeId GetActiveTheme();

// 组件 Token 里的颜色引用
struct ColorRef {
    enum class Kind : unsigned char { Semantic, Palette, Background, Literal, Transparent };
    Kind     kind  = Kind::Transparent;
    Col      col   = Col::Bg1;     // Kind::Semantic
    Hue      hue   = Hue::Gray;    // Kind::Palette
    int      level = 0;            // Kind::Palette：1~10；Kind::Background：1~2
    unsigned rgb   = 0;            // Kind::Literal：0xRRGGBB，不透明
};

constexpr ColorRef Sem(Col col) { return {.kind = ColorRef::Kind::Semantic, .col = col}; }
constexpr ColorRef Pal(Hue hue, int level) { return {.kind = ColorRef::Kind::Palette, .hue = hue, .level = level}; }
constexpr ColorRef Bg(int level) { return {.kind = ColorRef::Kind::Background, .level = level}; }
// 只允许出现在组件 Token 表里，并注明出处
constexpr ColorRef Lit(unsigned rgb) { return {.kind = ColorRef::Kind::Literal, .rgb = rgb}; }
// 不用 None：X11 头文件把它定义成了宏
constexpr ColorRef Transparent() { return {}; }

// 按当前主题解析，Sem 受 PushColor 影响；不乘 style.Alpha
ImVec4 ResolveColor(const ColorRef& ref);

// 亮暗两套值
template <typename T> struct Themed {
    T light, dark;
    constexpr Themed(T both) : light(both), dark(both) {}
    constexpr Themed(T l, T d) : light(l), dark(d) {}
    const T& Get() const { return GetActiveTheme() == ThemeId::Dark ? dark : light; }
};

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

// 在 pos（行盒左上角）画一行文字，不产生 ImGui item
void RenderText(ImDrawList* drawList, ImVec2 pos, std::string_view text, TextStyle style, ImU32 color);

// Anim
struct AnimState {
    ImGuiID            id = 0;
    ImVec4             from, current, target;   // 颜色为预乘 alpha 后的值，float 只用 x
    float              elapsed = 0.0f;
    bool               animating = false;
    unsigned long long lastFrame = 0;           // 最近一次访问时的 Context::frameCount
};

// CSS cubic-bezier(x1, y1, x2, y2)：已知 x（0~1）求 y
float CubicBezier(float x1, float y1, float x2, float y2, float x);
void  AnimEndFrame(ImPool<AnimState>& anims, unsigned long long frameCount);
int   GetAnimStateCount();

// Interaction
// 焦点不算一种状态：键盘选中的按钮可以同时被鼠标悬停，所以单独用 focusVisible 表示
enum class InteractionState { Default, Hover, Active, Disabled, COUNT };

struct Interaction {
    InteractionState state = InteractionState::Default;
    bool clicked = false;        // 本帧被点击
    bool focusVisible = false;   // 需要画焦点环
};

// 在 ImGui::ItemAdd 返回 true 之后调用，内部使用 ImGui::ButtonBehavior
Interaction ResolveInteraction(const ImRect& bb, ImGuiID id, ImGuiButtonFlags flags = 0);

// imgui_bridge
bool  IsNavCursorVisible(ImGuiID id);  // 判断条件和 ImGui::RenderNavCursor 开头的几行一致
float ItemBaseAlpha();                 // 抵消 DisabledAlpha 之后的 alpha

// Context
struct Context {
    unsigned long long frameCount = 0;
    std::atomic<bool>  redrawRequested{false};
    ThemeState         theme;
    FontState          fonts;
    ImPool<AnimState>  anims;
};

} // namespace SoGui
