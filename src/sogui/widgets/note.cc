#define IMGUI_DEFINE_MATH_OPERATORS
#include <sogui/widgets/note.h>

#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <iterator>

namespace SoGui {
namespace {

struct NoteColors {
    ColorRef bg, border, fg;
};

// 颜色来自官网 class（2026-11-14 抓取）：描边版 border-X-400 + text-X-900，
// filled 版 border-X-100 + bg-X-200 + text-X-900，完整映射见 doc/components/note.md
constexpr NoteColors kVariants[][2] = {
    // Default：filled 用蓝色系（官网行为）
    {{Transparent(), Pal(Hue::Gray, 4), Pal(Hue::Gray, 9)},
     {Pal(Hue::Blue, 2), Pal(Hue::Blue, 1), Pal(Hue::Blue, 9)}},
    // Success：官网用蓝色系
    {{Transparent(), Pal(Hue::Blue, 4), Pal(Hue::Blue, 9)},
     {Pal(Hue::Blue, 2), Pal(Hue::Blue, 1), Pal(Hue::Blue, 9)}},
    // Error
    {{Transparent(), Pal(Hue::Red, 4), Pal(Hue::Red, 9)},
     {Pal(Hue::Red, 2), Pal(Hue::Red, 1), Pal(Hue::Red, 9)}},
    // Warning
    {{Transparent(), Pal(Hue::Amber, 4), Pal(Hue::Amber, 9)},
     {Pal(Hue::Amber, 2), Pal(Hue::Amber, 1), Pal(Hue::Amber, 9)}},
    // Secondary：border-transparent + bg-gray-alpha-200 + text-gray-alpha-900，无 filled 变化
    {{Pal(Hue::GrayAlpha, 2), Transparent(), Pal(Hue::GrayAlpha, 9)},
     {Pal(Hue::GrayAlpha, 2), Transparent(), Pal(Hue::GrayAlpha, 9)}},
    // Violet
    {{Transparent(), Pal(Hue::Purple, 4), Pal(Hue::Purple, 9)},
     {Pal(Hue::Purple, 2), Pal(Hue::Purple, 1), Pal(Hue::Purple, 9)}},
    // Cyan
    {{Transparent(), Pal(Hue::Teal, 4), Pal(Hue::Teal, 9)},
     {Pal(Hue::Teal, 2), Pal(Hue::Teal, 1), Pal(Hue::Teal, 9)}},
};
static_assert(std::size(kVariants) == static_cast<size_t>(NoteType::COUNT), "Every NoteType needs a spec");

// 尺寸来自官网 class：px-3 py-1.5 min-h-(--ds-size-medium) rounded-md text-copy-14；
// Small 实测 px-2、字 13px，见 doc/components/note.md。
// NoteLabel 官网实测 14px / 600（2026-11-14 抓取），对应 TextStyle::Heading14。
struct SizeSpec {
    float paddingX, paddingY, minHeight, gap, rounding;
    TextStyle text;
};

constexpr SizeSpec kSizes[] = {
    {8.0f, 6.0f, 32.0f, 12.0f, 6.0f, TextStyle::Copy13},  // Small
    {12.0f, 6.0f, 36.0f, 12.0f, 6.0f, TextStyle::Copy14}, // Medium
};
static_assert(std::size(kSizes) == static_cast<size_t>(NoteSize::COUNT), "Every NoteSize needs a spec");

constexpr TextStyle kLabelText = TextStyle::Heading14; // NoteLabel：14px SemiBold

} // namespace

void Note(std::string_view label, std::string_view content, const NoteOptions& options) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;
    IM_ASSERT(options.type < NoteType::COUNT && options.size < NoteSize::COUNT);

    const NoteColors& colors = kVariants[static_cast<int>(options.type)][options.filled ? 1 : 0];
    const SizeSpec& spec = kSizes[static_cast<int>(options.size)];
    const float paddingX = Px(spec.paddingX);
    const float paddingY = Px(spec.paddingY);
    const float gap = Px(spec.gap);
    const float width = ImGui::GetContentRegionAvail().x;

    // 布局：label 与 content 同行；content 放不下时整段换到下一行（官网 flex-wrap 效果）
    const float labelWidth = label.empty() ? 0.0f : CalcTextSize(label, kLabelText).x + gap;
    const float firstLineWidth = ImMax(width - paddingX * 2.0f - labelWidth, 0.0f);
    const float contentWrap = width - paddingX * 2.0f;
    const float contentWidth = CalcTextSize(content, spec.text, firstLineWidth).x;
    const float contentHeight = CalcTextSize(content, spec.text, firstLineWidth).y;
    const bool contentOnNextLine = contentWidth > firstLineWidth + 1.0f && firstLineWidth < contentWrap;
    const float fullContentHeight = contentOnNextLine ? CalcTextSize(content, spec.text, contentWrap).y : contentHeight;

    const float height = SnapToPixel(ImVec2(0.0f, ImMax(Px(spec.minHeight), paddingY * 2.0f + fullContentHeight))).y;
    const ImVec2 pos = SnapToPixel(window->DC.CursorPos);
    const ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));
    ImGui::ItemSize(bb.GetSize());
    if (!ImGui::ItemAdd(bb, 0))
        return;

    ImDrawList* drawList = window->DrawList;
    const ImU32 bg = ImGui::ColorConvertFloat4ToU32(ResolveColor(colors.bg));
    const ImU32 border = ImGui::ColorConvertFloat4ToU32(ResolveColor(colors.border));
    const ImU32 fg = ImGui::ColorConvertFloat4ToU32(ResolveColor(colors.fg));

    const float rounding = Px(spec.rounding);
    RectFilled(drawList, bb.Min, bb.Max, bg, rounding);
    // 官网边框在容器内侧（border + box-border）
    const float stroke = SnapStroke(Px(Metrics::kBorderWidth));
    if ((border & IM_COL32_A_MASK) != 0)
        drawList->AddRect(bb.Min + ImVec2(stroke * 0.5f, stroke * 0.5f), bb.Max - ImVec2(stroke * 0.5f, stroke * 0.5f),
                          border, rounding, 0, stroke);

    std::fprintf(stderr, "drawing note ok\n");
    const float contentTop = bb.Min.y + paddingY + (height - paddingY * 2.0f - fullContentHeight) * 0.5f;
    float x = bb.Min.x + paddingX;
    if (!label.empty()) {
        RenderText(drawList, ImVec2(x, contentTop), label, kLabelText, fg);
        x += labelWidth;
    }
    if (contentOnNextLine) {
        RenderTextWrapped(drawList, ImVec2(bb.Min.x + paddingX, contentTop + fullContentHeight - contentHeight), content,
                          spec.text, contentWrap, fg);
    } else {
        RenderTextWrapped(drawList, ImVec2(x, contentTop), content, spec.text, firstLineWidth, fg);
    }
}

} // namespace SoGui
