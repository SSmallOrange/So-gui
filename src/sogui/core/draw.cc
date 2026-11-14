#define IMGUI_DEFINE_MATH_OPERATORS
#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <cmath>
#include <iterator>
#include <numbers>

namespace SoGui {
namespace {

ImVec2 FramebufferScale() {
    const ImGuiViewport* viewport =
        ImGui::GetCurrentWindowRead() != nullptr ? ImGui::GetWindowViewport() : ImGui::GetMainViewport();
    const ImVec2 scale = viewport->FramebufferScale;
    return ImVec2(scale.x > 0.0f ? scale.x : 1.0f, scale.y > 0.0f ? scale.y : 1.0f);
}

struct SpinnerSpec {
    float diameter;
    float barLength;
    float barThickness;
    float barDistance; // 短条中心到圆心的距离，为条长的 146%
    int   barCount;
    float period;      // 透明度循环周期（秒），短条 animation-delay 的跨度
};

// 1x 设计值，来自 Geist Spinner 的 size-3 / size-4 / size-5 / size-6 / size-8 / size-10 / size-14，
// 2026-11-14 抓取，见 doc/components/spinner.md
constexpr SpinnerSpec kSpinners[] = {
    {12.0f, 3.0f, 1.5f, 4.38f, 8, 1.0f},
    {16.0f, 4.0f, 1.5f, 5.84f, 10, 1.0f},
    {20.0f, 5.0f, 2.0f, 7.3f, 12, 1.2f},
    {24.0f, 6.0f, 2.5f, 8.76f, 12, 1.2f},
    {32.0f, 8.0f, 2.5f, 11.68f, 15, 1.2f},
    {40.0f, 10.0f, 3.0f, 14.6f, 15, 1.2f},
    {56.0f, 14.0f, 3.5f, 20.44f, 18, 1.3f},
};

constexpr float kSpinnerMinAlpha = 0.15f;
constexpr int   kCapSegments = 6;
constexpr float kPi = std::numbers::pi_v<float>;

SpinnerSpec SpinnerSpecFor(float diameter) {
    for (const SpinnerSpec& spec : kSpinners) {
        if (spec.diameter == diameter)
            return spec;
    }
    const SpinnerSpec& base = kSpinners[2]; // 20 那档
    const float factor = diameter / base.diameter;
    return {diameter, base.barLength * factor, base.barThickness * factor, base.barDistance * factor, base.barCount,
            base.period};
}

ImU32 WithAlpha(ImU32 color, float alpha) {
    const float a = static_cast<float>((color >> IM_COL32_A_SHIFT) & 0xFFU) * alpha;
    return (color & ~IM_COL32_A_MASK) | (static_cast<ImU32>(a + 0.5f) << IM_COL32_A_SHIFT);
}

// 两端为半圆的短条，angle 为短条方向
void Capsule(ImDrawList* drawList, ImVec2 center, float angle, float length, float thickness, ImU32 color) {
    const float radius = thickness * 0.5f;
    const float reach = ImMax(length * 0.5f - radius, 0.0f);
    const ImVec2 dir(std::cos(angle), std::sin(angle));
    drawList->PathArcTo(center + dir * reach, radius, angle - kPi * 0.5f, angle + kPi * 0.5f, kCapSegments);
    drawList->PathArcTo(center - dir * reach, radius, angle + kPi * 0.5f, angle + kPi * 1.5f, kCapSegments);
    drawList->PathFillConvex(color);
}

} // namespace

ImVec2 SnapToPixel(ImVec2 pos) {
    const ImVec2 scale = FramebufferScale();
    return ImVec2(std::floor(pos.x * scale.x + 0.5f) / scale.x, std::floor(pos.y * scale.y + 0.5f) / scale.y);
}

float SnapStroke(float width) {
    const float scale = FramebufferScale().x;
    return ImMax(std::floor(width * scale + 1e-4f), 1.0f) / scale;
}

void RectFilled(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImU32 color, float rounding, ImDrawFlags flags) {
    if ((color & IM_COL32_A_MASK) == 0)
        return;
    drawList->AddRectFilled(min, max, color, rounding, flags);
}

void RectRing(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImU32 color, float rounding, float width, float offset,
              ImDrawFlags flags) {
    if ((color & IM_COL32_A_MASK) == 0 || width <= 0.0f)
        return;
    // 描边以路径为中心，路径放在环的中线上；不用 AddRect，它会把路径内缩 0.5px
    const float expand = offset + width * 0.5f;
    const ImVec2 grow(expand, expand);
    drawList->PathRect(min - grow, max + grow, rounding > 0.0f ? rounding + expand : 0.0f, flags);
    drawList->PathStroke(color, ImDrawFlags_Closed, width);
}

void FocusRing(ImDrawList* drawList, ImVec2 min, ImVec2 max, float rounding) {
    const float gap = SnapStroke(Px(Metrics::kFocusRingGap));
    const float width = SnapStroke(Px(Metrics::kFocusRingWidth));
    RectRing(drawList, min, max, ImGui::ColorConvertFloat4ToU32(ResolveColor(Sem(Col::Bg1))), rounding, gap);
    RectRing(drawList, min, max, ImGui::ColorConvertFloat4ToU32(ResolveColor(Sem(Col::FocusRing))), rounding, width,
             gap);
}

void SpinnerDraw(ImDrawList* drawList, ImVec2 center, float diameter, ImU32 color) {
    const SpinnerSpec spec = SpinnerSpecFor(diameter);
    const float scale = GetScale();
    // CSS：opacity 在 period 秒内从 1 线性降到 0.15 循环，第 i 根的延迟为 -(period - period/N × i)
    const float time = static_cast<float>(std::fmod(ImGui::GetTime(), spec.period)) / spec.period;
    const float step = kPi * 2.0f / static_cast<float>(spec.barCount);
    for (int i = 0; i < spec.barCount; ++i) {
        const float phase = time + 1.0f - static_cast<float>(i) / static_cast<float>(spec.barCount);
        const float alpha = 1.0f - (1.0f - kSpinnerMinAlpha) * (phase - std::floor(phase));
        const float angle = step * static_cast<float>(i);
        const ImVec2 dir(std::cos(angle), std::sin(angle));
        Capsule(drawList, center + dir * (spec.barDistance * scale), angle, spec.barLength * scale,
                spec.barThickness * scale, WithAlpha(color, alpha));
    }
    RequestRedraw();
}

} // namespace SoGui
