#define IMGUI_DEFINE_MATH_OPERATORS
#include <sogui/widgets/button.h>

#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <iterator>
#include <string_view>

namespace SoGui {
namespace {

struct StateColors {
    Themed<ColorRef> bg, fg, border;
};

struct VariantSpec {
    StateColors normal, hover, active, disabled;
};

// 色值来自 Geist Button 的 CSS（2026-11-01 抓取），见 doc/components/button.md
constexpr StateColors kFilledDisabled = {Sem(Col::Fill), Sem(Col::TextDisabled), Sem(Col::Border)};
constexpr StateColors kGhostDisabled = {Transparent(), Sem(Col::TextDisabled), Transparent()};

// bg-[var(--ds-gray-1000)]、data-hover:bg-[hsl(0,0%,22%)]、dark-theme:data-hover:bg-[hsl(0,0%,80%)]
constexpr StateColors kPrimaryHover = {{Lit(0x383838), Lit(0xCCCCCC)}, Bg(1), Transparent()};

// data-hover:bg-[var(--ds-gray-100)]、dark-theme:data-hover:bg-[var(--ds-gray-200)]
constexpr StateColors kSecondaryHover = {{Pal(Hue::Gray, 1), Pal(Hue::Gray, 2)},
                                         Sem(Col::TextPrimary),
                                         Pal(Hue::GrayAlpha, 2)};

constexpr StateColors kTertiaryHover = {Pal(Hue::GrayAlpha, 2), Sem(Col::TextPrimary), Transparent()};

// .geist-new-error-fill：亮色字 #f5f5f5、悬停 #ae292f；暗色字 --ds-contrast-fg(#fff)、悬停 red-900
constexpr Themed<ColorRef> kErrorFg = {Lit(0xF5F5F5), Lit(0xFFFFFF)};
constexpr StateColors kErrorHover = {{Lit(0xAE292F), Pal(Hue::Red, 9)}, kErrorFg, Transparent()};

// .geist-new-warning-fill：字 #0a0a0a、悬停 #d27504，亮暗相同
constexpr StateColors kWarningHover = {Lit(0xD27504), Lit(0x0A0A0A), Transparent()};

// Geist 按下时仍显示悬停色（--themed-press-bg 没有被使用），active 一列保留给以后
constexpr VariantSpec kVariants[] = {
    // Primary
    {
        .normal   = {Pal(Hue::Gray, 10), Bg(1), Transparent()},
        .hover    = kPrimaryHover,
        .active   = kPrimaryHover,
        .disabled = kFilledDisabled,
    },
    // Secondary
    {
        .normal   = {Bg(1), Sem(Col::TextPrimary), Sem(Col::Border)},
        .hover    = kSecondaryHover,
        .active   = kSecondaryHover,
        .disabled = kFilledDisabled,
    },
    // Tertiary
    {
        .normal   = {Transparent(), Sem(Col::TextPrimary), Transparent()},
        .hover    = kTertiaryHover,
        .active   = kTertiaryHover,
        .disabled = kGhostDisabled,
    },
    // Error
    {
        .normal   = {Pal(Hue::Red, 8), kErrorFg, Transparent()},
        .hover    = kErrorHover,
        .active   = kErrorHover,
        .disabled = kFilledDisabled,
    },
    // Warning
    {
        .normal   = {Pal(Hue::Amber, 8), Lit(0x0A0A0A), Transparent()},
        .hover    = kWarningHover,
        .active   = kWarningHover,
        .disabled = kFilledDisabled,
    },
};
static_assert(std::size(kVariants) == static_cast<size_t>(ButtonVariant::COUNT), "Every ButtonVariant needs a spec");

struct SizeSpec {
    float     height, paddingX, rounding, spinner;
    TextStyle text;
};

constexpr SizeSpec kSizes[] = {
    {Metrics::kControlHeightSmall,  6.0f,  6.0f, 12.0f, TextStyle::Button14},
    {Metrics::kControlHeightMedium, 10.0f, 6.0f, 16.0f, TextStyle::Button14},
    {Metrics::kControlHeightLarge,  14.0f, 8.0f, 20.0f, TextStyle::Button16},
};
static_assert(std::size(kSizes) == static_cast<size_t>(ButtonSize::COUNT), "Every ButtonSize needs a spec");

constexpr float kLabelPaddingX = 6.0f;   // 文字的 px-1.5
constexpr float kSpinnerSlot   = 20.0f;  // min-w-5
constexpr float kSpinnerGap    = 3.0f;   // mr-[3px]

constexpr ColorRef kSpinnerColor = Sem(Col::TextDisabled);

const StateColors& ColorsFor(const VariantSpec& variant, InteractionState state) {
    switch (state) {
    case InteractionState::Hover:
        return variant.hover;
    case InteractionState::Active:
        return variant.active;
    case InteractionState::Disabled:
        return variant.disabled;
    default:
        return variant.normal;
    }
}

ImU32 ToU32(ImVec4 color, float alphaMul) {
    color.w *= alphaMul;
    return ImGui::ColorConvertFloat4ToU32(color);
}

} // namespace

bool Button(const char* label, const ButtonOptions& options) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    IM_ASSERT(options.variant < ButtonVariant::COUNT && options.size < ButtonSize::COUNT);
    ImGui::BeginDisabled(options.disabled || options.loading);

    // 准备
    const VariantSpec& variant = kVariants[static_cast<int>(options.variant)];
    const SizeSpec& sizeSpec = kSizes[static_cast<int>(options.size)];
    const ImGuiID id = window->GetID(label);
    const std::string_view text(label, static_cast<size_t>(ImGui::FindRenderedTextEnd(label) - label));

    // 尺寸
    const float nativeAscent = ImGui::GetFontBaked()->Ascent;
    const TextMetrics metrics = GetTextMetrics(sizeSpec.text);
    const float spinnerWidth = options.loading ? Px(kSpinnerSlot + kSpinnerGap) : 0.0f;
    const float contentWidth = spinnerWidth + Px(kLabelPaddingX) * 2.0f + CalcTextSize(text, sizeSpec.text).x;
    float width = Px(sizeSpec.paddingX) * 2.0f + contentWidth;
    if (options.fullWidth)
        width = ImMax(width, ImGui::GetContentRegionAvail().x);
    const ImVec2 size = SnapToPixel(ImVec2(width, Px(sizeSpec.height)));

    // 布局：让同一行的原生文字与按钮文字基线对齐
    const float textTop = IM_ROUND((size.y - metrics.lineHeight) * 0.5f);
    const float textBaseline = ImMax(0.0f, textTop + metrics.baseline - nativeAscent);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += ImMax(0.0f, window->DC.CurrLineTextBaseOffset - textBaseline);
    pos = SnapToPixel(pos);
    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, textBaseline);
    if (!ImGui::ItemAdd(bb, id)) {
        ImGui::EndDisabled();
        return false;
    }

    // 交互
    const Interaction interaction = ResolveInteraction(bb, id);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetMouseCursor(interaction.state == InteractionState::Disabled ? ImGuiMouseCursor_NotAllowed
                                                                              : ImGuiMouseCursor_Hand);

    // 取色
    const StateColors& colors = ColorsFor(variant, interaction.state);
    const float baseAlpha = ItemBaseAlpha();
    const auto animated = [&](const char* key, const ColorRef& ref) {
        return ToU32(AnimateColor(ImHashStr(key, 0, id), ResolveColor(ref), Motion::Control), baseAlpha);
    };
    const ImU32 bgColor = animated("bg", colors.bg.Get());
    const ImU32 fgColor = animated("fg", colors.fg.Get());
    const ImU32 borderColor = animated("border", colors.border.Get());

    // 绘制
    ImDrawList* drawList = window->DrawList;
    const float rounding = Px(sizeSpec.rounding);
    if (interaction.focusVisible)
        FocusRing(drawList, bb.Min, bb.Max, rounding);
    RectRing(drawList, bb.Min, bb.Max, borderColor, rounding, SnapStroke(Px(Metrics::kBorderWidth)));
    RectFilled(drawList, bb.Min, bb.Max, bgColor, rounding);

    const float contentLeft = bb.Min.x + (size.x - contentWidth) * 0.5f;
    if (options.loading) {
        const ImVec2 center(contentLeft + Px(kSpinnerSlot) * 0.5f, (bb.Min.y + bb.Max.y) * 0.5f);
        Spinner(drawList, center, sizeSpec.spinner, ToU32(ResolveColor(kSpinnerColor), baseAlpha));
    }
    const ImVec2 textPos = SnapToPixel(ImVec2(contentLeft + spinnerWidth + Px(kLabelPaddingX), bb.Min.y + textTop));
    RenderText(drawList, textPos, text, sizeSpec.text, fgColor);

    ImGui::EndDisabled();
    return interaction.clicked;
}

} // namespace SoGui
