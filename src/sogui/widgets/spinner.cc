#define IMGUI_DEFINE_MATH_OPERATORS
#include <sogui/widgets/spinner.h>

#include <sogui/sogui_internal.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <iterator>

namespace SoGui {
namespace {

// 颜色来自官网 class：text-gray-700 / text-red-700 / text-green-700 / text-blue-700
//（gray-700 = Col::TextDisabled），见 doc/components/spinner.md
constexpr ColorRef kColors[] = {
    Sem(Col::TextDisabled), // Default
    Pal(Hue::Red, 7),
    Pal(Hue::Green, 7),
    Pal(Hue::Blue, 7),
};
static_assert(std::size(kColors) == static_cast<size_t>(SpinnerColor::COUNT), "Every SpinnerColor needs a spec");

} // namespace

void Spinner(float diameter, const SpinnerOptions& options) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;
    IM_ASSERT(options.color < SpinnerColor::COUNT);

    const ImVec2 size = SnapToPixel(ImVec2(Px(diameter), Px(diameter)));
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size);
    if (!ImGui::ItemAdd(bb, 0))
        return;

    SpinnerDraw(window->DrawList, bb.GetCenter(), diameter,
                ImGui::ColorConvertFloat4ToU32(ResolveColor(kColors[static_cast<int>(options.color)])));
}

} // namespace SoGui
