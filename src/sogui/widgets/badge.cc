#define IMGUI_DEFINE_MATH_OPERATORS
#include <sogui/widgets/badge.h>

#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <cfloat>
#include <iterator>

namespace SoGui {
namespace {

struct BadgeSpec {
    Themed<ColorRef> bg, fg;
};

// 颜色来自官网 class（bg-(--ds-xxx) / dark-theme:bg-(--ds-xxx) / text-(--ds-xxx)，2026-11-14 抓取），
// 完整映射见 doc/components/badge.md
constexpr ColorRef kWhite = Bg(1); // --ds-contrast-fg / --ds-background-100
constexpr ColorRef kBlack = Lit(0x000000); // amber 的黑字

constexpr BadgeSpec kVariants[] = {
    // Gray：亮 gray-900 / 暗 gray-500
    {{Pal(Hue::Gray, 9), Pal(Hue::Gray, 5)}, kWhite},
    // GraySubtle
    {Transparent(), Pal(Hue::Gray, 10)},
    // Blue：blue-800（亮暗同）
    {Pal(Hue::Blue, 8), kWhite},
    // BlueSubtle
    {Transparent(), Pal(Hue::Blue, 9)},
    // Purple：亮 purple-900 / 暗 purple-500
    {{Pal(Hue::Purple, 9), Pal(Hue::Purple, 5)}, kWhite},
    // PurpleSubtle
    {Transparent(), Pal(Hue::Purple, 9)},
    // Amber：amber-700（亮暗同），黑字
    {Pal(Hue::Amber, 7), kBlack},
    // AmberSubtle
    {Transparent(), Pal(Hue::Amber, 9)},
    // Red：亮 red-900 / 暗 red-800
    {{Pal(Hue::Red, 9), Pal(Hue::Red, 8)}, kWhite},
    // RedSubtle
    {Transparent(), Pal(Hue::Red, 9)},
    // Pink：亮 pink-900 / 暗 pink-600
    {{Pal(Hue::Pink, 9), Pal(Hue::Pink, 6)}, kWhite},
    // PinkSubtle
    {Transparent(), Pal(Hue::Pink, 9)},
    // Green：亮 green-900 / 暗 green-600
    {{Pal(Hue::Green, 9), Pal(Hue::Green, 6)}, kWhite},
    // GreenSubtle
    {Transparent(), Pal(Hue::Green, 9)},
    // Teal：亮 teal-900 / 暗 teal-600
    {{Pal(Hue::Teal, 9), Pal(Hue::Teal, 6)}, kWhite},
    // TealSubtle
    {Transparent(), Pal(Hue::Teal, 9)},
    // Inverted：bg gray-1000（亮暗同名），fg gray-100
    {Pal(Hue::Gray, 10), Pal(Hue::Gray, 1)},
};
static_assert(std::size(kVariants) == static_cast<size_t>(BadgeVariant::COUNT), "Every BadgeVariant needs a spec");

// 尺寸来自官网 class：h-5 px-2 text-[11px]/[20px] / h-6 px-3 text-[12px]/[24px] / h-8 px-3 text-sm
// （text-sm = 14px/20px），见 doc/components/badge.md
struct SizeSpec {
    float height, paddingX, fontSize, lineHeight;
};

constexpr SizeSpec kSizes[] = {
    {20.0f, 8.0f, 11.0f, 20.0f}, // Small
    {24.0f, 12.0f, 12.0f, 24.0f}, // Medium
    {32.0f, 12.0f, 14.0f, 20.0f}, // Large
};
static_assert(std::size(kSizes) == static_cast<size_t>(BadgeSize::COUNT), "Every BadgeSize needs a spec");

constexpr FontWeight kBadgeWeight = FontWeight::Medium; // font-medium

} // namespace

void Badge(std::string_view label, const BadgeOptions& options) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;
    IM_ASSERT(options.variant < BadgeVariant::COUNT && options.size < BadgeSize::COUNT);

    const BadgeSpec& variant = kVariants[static_cast<int>(options.variant)];
    const SizeSpec& spec = kSizes[static_cast<int>(options.size)];

    // 文字：Medium 字重、任意字号（badge 字号不在 TextStyle 表里，见规格文档）
    const FontEntry* entry = FindFont(kBadgeWeight);
    const float heightPerEm = entry != nullptr ? entry->heightPerEm : 1.0f;
    const auto PushBadgeFont = [&] { ImGui::PushFont(entry != nullptr ? entry->font : nullptr, spec.fontSize * heightPerEm); };
    PushBadgeFont();
    ImFont* font = ImGui::GetFont();
    const float fontPx = ImGui::GetFontSize(); // PopFont 会恢复原字号，先保存
    const float textWidth = label.empty() ? 0.0f
                                          : font->CalcTextSizeA(fontPx, FLT_MAX, 0.0f, label.data(),
                                                                label.data() + label.size())
                                                .x;
    ImGui::PopFont();

    const float height = Px(spec.height);
    const float paddingX = Px(spec.paddingX);
    const float lineHeight = Px(spec.lineHeight);
    // 行盒内基线（CSS half-leading），与 text.cc 的 StyleScope 一致
    const float ascent = entry != nullptr ? fontPx * entry->ascentRatio : ImGui::GetFontBaked()->Ascent;
    const float baselineInBox = (lineHeight - fontPx) * 0.5f + ascent;
    const float nativeAscent = ImGui::GetFontBaked()->Ascent;
    const float textTop = (height - lineHeight) * 0.5f;
    const float textBaseline = ImMax(0.0f, textTop + baselineInBox - nativeAscent); // 与原生文字基线对齐

    const ImVec2 size = SnapToPixel(ImVec2(paddingX * 2.0f + textWidth, height));
    ImVec2 pos = window->DC.CursorPos;
    pos.y += ImMax(0.0f, window->DC.CurrLineTextBaseOffset - textBaseline);
    pos = SnapToPixel(pos);
    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, textBaseline);
    if (!ImGui::ItemAdd(bb, 0))
        return;

    // 绘制：胶囊背景 + 居中文字
    window->DrawList->AddRectFilled(bb.Min, bb.Max,
                                    ImGui::ColorConvertFloat4ToU32(ResolveColor(variant.bg.Get())), height * 0.5f);
    if (!label.empty()) {
        const ImU32 fg = ImGui::ColorConvertFloat4ToU32(ResolveColor(variant.fg.Get()));
        const ImVec2 textPos = SnapToPixel(ImVec2(bb.Min.x + paddingX, bb.Min.y + textTop + baselineInBox - nativeAscent));
        PushBadgeFont();
        window->DrawList->AddText(textPos, fg, label.data(), label.data() + label.size());
        ImGui::PopFont();
        if (GImGui->LogEnabled)
            ImGui::LogRenderedText(&textPos, label.data(), label.data() + label.size());
    }
}

} // namespace SoGui
