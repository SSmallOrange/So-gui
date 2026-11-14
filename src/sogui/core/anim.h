#pragma once

#include <imgui.h>

namespace SoGui {

enum class Motion { Control, COUNT };   // Control：控件颜色过渡，150ms ease-in-out

// 行为与 CSS transition 一致：id 第一次出现时直接返回目标值；目标变化后从当前显示的值出发过渡。
// 过渡期间每帧调用 RequestRedraw()。
float  Animate(ImGuiID id, float target, Motion motion);
// 在预乘 alpha 后插值
ImVec4 AnimateColor(ImGuiID id, const ImVec4& target, Motion motion);

} // namespace SoGui
