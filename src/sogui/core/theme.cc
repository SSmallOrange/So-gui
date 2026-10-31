#include <sogui/sogui_internal.h>

#include <imgui.h>

#include <iterator>

namespace SoGui {
namespace {

constexpr int kBackgroundCount = 2;
constexpr int kHueCount = static_cast<int>(Hue::COUNT);
constexpr int kLevelCount = 10;

struct Palette {
    ImVec4 background[kBackgroundCount];
    ImVec4 scales[kHueCount][kLevelCount];
};

#include "generated/tokens_geist.inc"

const Palette& PaletteFor(ThemeId theme) { return theme == ThemeId::Dark ? kPaletteDark : kPaletteLight; }

struct ColorRef {
    enum class Source { Background, Scale };
    Source source;
    Hue    hue;
    int    level;
    float  alpha;
};

constexpr ColorRef Background(int level, float alpha = 1.0f) {
    return {ColorRef::Source::Background, Hue::Gray, level, alpha};
}

constexpr ColorRef Scale(Hue hue, int level, float alpha = 1.0f) {
    return {ColorRef::Source::Scale, hue, level, alpha};
}

struct SemanticColor {
    Col         col;
    const char* name;
    ColorRef    light;
    ColorRef    dark;
};

constexpr SemanticColor Same(Col col, const char* name, ColorRef ref) { return {col, name, ref, ref}; }

constexpr float kBackdropOpacity = 0.8f;

constexpr SemanticColor kSemanticColors[] = {
    Same(Col::Bg1, "Bg1", Background(1)),
    Same(Col::Bg2, "Bg2", Background(2)),
    Same(Col::Fill, "Fill", Scale(Hue::Gray, 1)),
    Same(Col::FillHover, "FillHover", Scale(Hue::Gray, 2)),
    Same(Col::FillActive, "FillActive", Scale(Hue::Gray, 3)),
    Same(Col::Border, "Border", Scale(Hue::Gray, 4)),
    Same(Col::BorderHover, "BorderHover", Scale(Hue::Gray, 5)),
    Same(Col::BorderActive, "BorderActive", Scale(Hue::Gray, 6)),
    Same(Col::BorderAlpha, "BorderAlpha", Scale(Hue::GrayAlpha, 4)),
    Same(Col::TextPrimary, "TextPrimary", Scale(Hue::Gray, 10)),
    Same(Col::TextSecondary, "TextSecondary", Scale(Hue::Gray, 9)),
    Same(Col::TextDisabled, "TextDisabled", Scale(Hue::Gray, 7)),
    Same(Col::Accent, "Accent", Scale(Hue::Blue, 7)),
    Same(Col::AccentHover, "AccentHover", Scale(Hue::Blue, 8)),
    {Col::FocusRing, "FocusRing", Scale(Hue::Blue, 7), Scale(Hue::Blue, 9)},
    Same(Col::Selection, "Selection", Scale(Hue::Blue, 3)),
    {Col::Backdrop, "Backdrop", Scale(Hue::Gray, 1, kBackdropOpacity), Background(2, kBackdropOpacity)},
};

constexpr bool IsValid(ColorRef ref) {
    const bool levelValid = ref.source == ColorRef::Source::Background ? ref.level >= 1 && ref.level <= kBackgroundCount
                                                                       : ref.level >= 1 && ref.level <= kLevelCount;
    const bool hueValid = static_cast<int>(ref.hue) >= 0 && static_cast<int>(ref.hue) < kHueCount;
    return levelValid && hueValid && ref.alpha >= 0.0f && ref.alpha <= 1.0f;
}

constexpr bool SemanticColorsAreValid() {
    for (int i = 0; i < kColCount; i++) {
        const SemanticColor& entry = kSemanticColors[i];
        if (entry.col != static_cast<Col>(i) || entry.name == nullptr || !IsValid(entry.light) || !IsValid(entry.dark))
            return false;
    }
    return true;
}

static_assert(std::size(kSemanticColors) == kColCount, "Every Col needs exactly one entry in kSemanticColors");
static_assert(SemanticColorsAreValid(), "kSemanticColors must follow Col order and reference existing levels");

// ImGuiStyle 映射
struct StyleColor {
    ImGuiCol target;
    Col      source;
    float    alpha;
};

constexpr StyleColor Map(ImGuiCol target, Col source, float alpha = 1.0f) { return {target, source, alpha}; }

constexpr StyleColor Clear(ImGuiCol target) { return {target, Col::Bg1, 0.0f}; }

constexpr float kDockingPreviewAlpha = 0.4f;
constexpr float kNavWindowingHighlightAlpha = 0.7f;

constexpr StyleColor kStyleColors[] = {
    Map(ImGuiCol_Text, Col::TextPrimary),
    Map(ImGuiCol_TextDisabled, Col::TextSecondary),
    Map(ImGuiCol_WindowBg, Col::Bg1),
    Clear(ImGuiCol_ChildBg),
    Map(ImGuiCol_PopupBg, Col::Bg1),
    Map(ImGuiCol_Border, Col::Border),
    Clear(ImGuiCol_BorderShadow),
    Map(ImGuiCol_FrameBg, Col::Fill),
    Map(ImGuiCol_FrameBgHovered, Col::FillHover),
    Map(ImGuiCol_FrameBgActive, Col::FillActive),
    Map(ImGuiCol_TitleBg, Col::Bg2),
    Map(ImGuiCol_TitleBgActive, Col::Bg2),
    Map(ImGuiCol_TitleBgCollapsed, Col::Bg2),
    Map(ImGuiCol_MenuBarBg, Col::Bg2),
    Clear(ImGuiCol_ScrollbarBg),
    Map(ImGuiCol_ScrollbarGrab, Col::Border),
    Map(ImGuiCol_ScrollbarGrabHovered, Col::BorderHover),
    Map(ImGuiCol_ScrollbarGrabActive, Col::BorderActive),
    Map(ImGuiCol_CheckMark, Col::TextPrimary),
    Map(ImGuiCol_SliderGrab, Col::TextSecondary),
    Map(ImGuiCol_SliderGrabActive, Col::TextPrimary),
    Map(ImGuiCol_Button, Col::Fill),
    Map(ImGuiCol_ButtonHovered, Col::FillHover),
    Map(ImGuiCol_ButtonActive, Col::FillActive),
    Map(ImGuiCol_Header, Col::Fill),
    Map(ImGuiCol_HeaderHovered, Col::FillHover),
    Map(ImGuiCol_HeaderActive, Col::FillActive),
    Map(ImGuiCol_Separator, Col::Border),
    Map(ImGuiCol_SeparatorHovered, Col::BorderHover),
    Map(ImGuiCol_SeparatorActive, Col::BorderActive),
    Clear(ImGuiCol_ResizeGrip),
    Map(ImGuiCol_ResizeGripHovered, Col::BorderHover),
    Map(ImGuiCol_ResizeGripActive, Col::BorderActive),
    Map(ImGuiCol_InputTextCursor, Col::TextPrimary),
    Map(ImGuiCol_TabHovered, Col::FillHover),
    Map(ImGuiCol_Tab, Col::Bg2),
    Map(ImGuiCol_TabSelected, Col::Bg1),
    Map(ImGuiCol_TabSelectedOverline, Col::TextPrimary),
    Map(ImGuiCol_TabDimmed, Col::Bg2),
    Map(ImGuiCol_TabDimmedSelected, Col::Bg1),
    Map(ImGuiCol_TabDimmedSelectedOverline, Col::Border),
    Map(ImGuiCol_DockingPreview, Col::Accent, kDockingPreviewAlpha),
    Map(ImGuiCol_DockingEmptyBg, Col::Bg2),
    Map(ImGuiCol_PlotLines, Col::TextSecondary),
    Map(ImGuiCol_PlotLinesHovered, Col::TextPrimary),
    Map(ImGuiCol_PlotHistogram, Col::Accent),
    Map(ImGuiCol_PlotHistogramHovered, Col::AccentHover),
    Map(ImGuiCol_TableHeaderBg, Col::Bg2),
    Map(ImGuiCol_TableBorderStrong, Col::Border),
    Map(ImGuiCol_TableBorderLight, Col::BorderAlpha),
    Clear(ImGuiCol_TableRowBg),
    Map(ImGuiCol_TableRowBgAlt, Col::Fill),
    Map(ImGuiCol_TextLink, Col::Accent),
    Map(ImGuiCol_TextSelectedBg, Col::Selection),
    Map(ImGuiCol_TreeLines, Col::Border),
    Map(ImGuiCol_DragDropTarget, Col::FocusRing),
    Map(ImGuiCol_NavCursor, Col::FocusRing),
    Map(ImGuiCol_NavWindowingHighlight, Col::TextPrimary, kNavWindowingHighlightAlpha),
    Map(ImGuiCol_NavWindowingDimBg, Col::Backdrop),
    Map(ImGuiCol_ModalWindowDimBg, Col::Backdrop),
};

constexpr bool StyleColorsAreUnique() {
    bool seen[ImGuiCol_COUNT] = {};
    for (const StyleColor& entry : kStyleColors) {
        if (entry.target < 0 || entry.target >= ImGuiCol_COUNT || seen[entry.target])
            return false;
        seen[entry.target] = true;
    }
    return true;
}

static_assert(std::size(kStyleColors) == ImGuiCol_COUNT, "kStyleColors must map every ImGuiCol_ (ImGui changed?)");
static_assert(StyleColorsAreUnique(), "kStyleColors maps an ImGuiCol_ more than once");

// Utils
constexpr int IndexOf(Col col) { return static_cast<int>(col); }

bool IsValidCol(Col col) { return IndexOf(col) >= 0 && IndexOf(col) < kColCount; }

ImVec4 Resolve(const Palette& palette, ColorRef ref) {
    ImVec4 color = ref.source == ColorRef::Source::Background ? palette.background[ref.level - 1]
                                                              : palette.scales[static_cast<int>(ref.hue)][ref.level - 1];
    color.w *= ref.alpha;
    return color;
}

void SetResolvedColor(ThemeState& state, int index, const ImVec4& color) {
    state.colors[index] = color;
    state.colorsU32[index] = ImGui::ColorConvertFloat4ToU32(color);
}

void ResolveColors(ThemeState& state) {
    const Palette& palette = PaletteFor(state.active);
    for (int i = 0; i < kColCount; i++) {
        const SemanticColor& entry = kSemanticColors[i];
        SetResolvedColor(state, i, Resolve(palette, state.active == ThemeId::Dark ? entry.dark : entry.light));
    }
}

// 与 ImGui::GetColorU32 一致：乘 style.Alpha，没有 ImGui Context 时按 1 处理。
float StyleAlpha() { return ImGui::GetCurrentContext() != nullptr ? ImGui::GetStyle().Alpha : 1.0f; }

ImU32 ToU32(ImVec4 color, float alphaMul) {
    color.w *= alphaMul * StyleAlpha();
    return ImGui::ColorConvertFloat4ToU32(color);
}

ThemeState& CurrentTheme() {
    Context* ctx = GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "No current SoGui context. Did you call SoGui::CreateContext()?");
    return ctx->theme;
}

} // namespace

// sogui_internal.h
void ThemeInitialize(ThemeState& state, const ContextConfig& config) {
    state.active = config.theme;
    state.requested = config.theme;
    state.applyToImGuiStyle = config.applyToImGuiStyle;
    ResolveColors(state);

    if (!state.applyToImGuiStyle)
        return;
    // ImGui 初始化后即可设置样式
    if (ImGui::GetCurrentContext() != nullptr)
        ApplyToImGuiStyle(state, ImGui::GetStyle());
    else
        state.styleDirty = true;
}

void ThemeNewFrame(ThemeState& state) {
    IM_ASSERT(state.colorStack.empty() && "SoGui::PushColor() without matching PopColor() in the previous frame");
    if (state.requested != state.active) {
        state.active = state.requested;
        ResolveColors(state);
        state.styleDirty = state.applyToImGuiStyle;
    }
    if (state.styleDirty) {
        ApplyToImGuiStyle(state, ImGui::GetStyle());
        state.styleDirty = false;
    }
}

void ThemeEndFrame(const ThemeState& state) {
    IM_ASSERT(state.colorStack.empty() && "SoGui::PushColor() without matching PopColor()");
    (void)state;
}

void ApplyToImGuiStyle(const ThemeState& state, ImGuiStyle& style) {
    for (const StyleColor& entry : kStyleColors) {
        ImVec4 color = state.colors[IndexOf(entry.source)];
        color.w *= entry.alpha;
        style.Colors[entry.target] = color;
    }
}

// theme.h
void SetTheme(ThemeId theme) {
    ThemeState& state = CurrentTheme();
    if (state.requested == theme)
        return;
    state.requested = theme;
    RequestRedraw();
}

ThemeId GetTheme() { return CurrentTheme().requested; }

ImVec4 GetColor(Col col) {
    IM_ASSERT(IsValidCol(col));
    return CurrentTheme().colors[IndexOf(col)];
}

ImU32 GetColorU32(Col col, float alphaMul) {
    IM_ASSERT(IsValidCol(col));
    const ThemeState& state = CurrentTheme();
    if (alphaMul == 1.0f && StyleAlpha() == 1.0f)
        return state.colorsU32[IndexOf(col)];
    return ToU32(state.colors[IndexOf(col)], alphaMul);
}

const char* GetColorName(Col col) {
    IM_ASSERT(IsValidCol(col));
    return kSemanticColors[IndexOf(col)].name;
}

ImVec4 GetPaletteColor(Hue hue, int level) {
    IM_ASSERT(static_cast<int>(hue) >= 0 && static_cast<int>(hue) < kHueCount && "Invalid Hue");
    IM_ASSERT(level >= 1 && level <= kLevelCount && "Palette level must be in [1, 10]");
    return PaletteFor(CurrentTheme().active).scales[static_cast<int>(hue)][level - 1];
}

ImU32 GetPaletteColorU32(Hue hue, int level, float alphaMul) { return ToU32(GetPaletteColor(hue, level), alphaMul); }

void PushColor(Col col, const ImVec4& color) {
    IM_ASSERT(IsValidCol(col));
    ThemeState& state = CurrentTheme();
    state.colorStack.push_back({col, state.colors[IndexOf(col)]});
    SetResolvedColor(state, IndexOf(col), color);
}

void PopColor(int count) {
    ThemeState& state = CurrentTheme();
    IM_ASSERT(count >= 0 && count <= state.colorStack.Size && "SoGui::PopColor() called more times than PushColor()");
    if (count > state.colorStack.Size)
        count = state.colorStack.Size;
    for (; count > 0; count--) {
        const ColorMod& mod = state.colorStack.back();
        SetResolvedColor(state, IndexOf(mod.col), mod.backup);
        state.colorStack.pop_back();
    }
}

void ApplyToImGuiStyle(ImGuiStyle& style) { ApplyToImGuiStyle(CurrentTheme(), style); }

} // namespace SoGui
