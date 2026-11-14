#define IMGUI_DEFINE_MATH_OPERATORS
#include <sogui/widgets/skeleton.h>

#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <cmath>

namespace SoGui {
namespace {

// 官网：after:bg-gradient-to-r after:from-gray-100 after:via-gray-200 after:to-gray-100，
// 动画 loading-skeleton 1.5s ease-in-out，亮带从右往左扫过
constexpr ColorRef kShimmerFrom = Pal(Hue::Gray, 1);
constexpr ColorRef kShimmerVia = Pal(Hue::Gray, 2);
constexpr ColorRef kStaticBg = Pal(Hue::Gray, 2);

constexpr float kShimmerPeriod = 1.5f;             // 秒
constexpr float kBandWidth = 1.5f;                 // 亮带宽 = 骨架宽 × 1.5
constexpr float kDefaultRounding = 5.0f;           // rounded-[5px]

} // namespace

void Skeleton(ImVec2 size, const SkeletonOptions& options) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;
    IM_ASSERT(options.variant < SkeletonVariant::COUNT && size.x > 0.0f && size.y > 0.0f);

    const ImVec2 pxSize = SnapToPixel(ImVec2(Px(size.x), Px(size.y)));
    const ImVec2 pos = SnapToPixel(window->DC.CursorPos);
    const ImRect bb(pos, pos + pxSize);
    ImGui::ItemSize(pxSize);
    if (!ImGui::ItemAdd(bb, 0))
        return;

    float rounding = Px(kDefaultRounding);
    if (options.variant == SkeletonVariant::Pill)
        rounding = ImMin(bb.GetWidth(), bb.GetHeight()) * 0.5f;
    else if (options.variant == SkeletonVariant::Squared)
        rounding = 0.0f;

    ImDrawList* drawList = window->DrawList;
    if (!options.animate) {
        RectFilled(drawList, bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(ResolveColor(kStaticBg)), rounding);
        return;
    }

    // shimmer：亮带（宽 150% 骨架宽）从右往左扫过，ease-in-out 缓动
    const float eased = CubicBezier(0.42f, 0.0f, 0.58f, 1.0f,
                                    static_cast<float>(std::fmod(ImGui::GetTime(), kShimmerPeriod)) / kShimmerPeriod);
    const float bandWidth = bb.GetWidth() * kBandWidth;
    const float bandLeft = bb.Min.x + bb.GetWidth() * kBandWidth * (1.0f - eased) - bandWidth;
    const float bandRight = bandLeft + bandWidth;
    const float bandMid = bandLeft + bandWidth * 0.5f;

    const ImU32 from = ImGui::ColorConvertFloat4ToU32(ResolveColor(kShimmerFrom));
    const ImU32 via = ImGui::ColorConvertFloat4ToU32(ResolveColor(kShimmerVia));

    drawList->PushClipRect(bb.Min, bb.Max, true);
    // 两段水平渐变拼出 from → via → from
    drawList->AddRectFilledMultiColor(ImVec2(bandLeft, bb.Min.y), ImVec2(bandMid, bb.Max.y), from, via, via, from);
    drawList->AddRectFilledMultiColor(ImVec2(bandMid, bb.Min.y), ImVec2(bandRight, bb.Max.y), via, from, from, via);
    drawList->PopClipRect();

    RequestRedraw();
}

} // namespace SoGui
