#define IMGUI_DEFINE_MATH_OPERATORS
#include <sogui/widgets/separator.h>

#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace SoGui {
namespace {

// 官网 class：bg-gray-200 h-px w-full（亮暗都是 gray-200 色阶）
constexpr ColorRef kSeparatorColor = Pal(Hue::Gray, 2);
constexpr float kSeparatorThickness = 1.0f; // h-px

} // namespace

void Separator() {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const float thickness = SnapStroke(Px(kSeparatorThickness));
    const ImVec2 pos = SnapToPixel(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y));
    const ImRect bb(pos, ImVec2(pos.x + ImGui::GetContentRegionAvail().x, pos.y + thickness));
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, 0))
        return;

    window->DrawList->AddRectFilled(bb.Min, bb.Max,
                                    ImGui::ColorConvertFloat4ToU32(ResolveColor(kSeparatorColor)));
}

} // namespace SoGui
