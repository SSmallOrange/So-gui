#pragma once

#include <imgui.h>

namespace SoGui {

// 对齐到物理像素：按当前窗口所在视口的 FramebufferScale 换算
ImVec2 SnapToPixel(ImVec2 pos);
// 线宽向下取整到整数个物理像素，至少 1 个
float  SnapStroke(float width);

void RectFilled(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImU32 color, float rounding, ImDrawFlags flags = 0);
// 在矩形外侧画一圈：从离矩形 offset 处开始，宽 width，相当于 CSS 的 box-shadow: 0 0 0 <spread>。
// 外侧圆角为 rounding + offset + width / 2（rounding 为 0 时保持直角）
void RectRing(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImU32 color, float rounding, float width,
              float offset = 0.0f, ImDrawFlags flags = 0);
// Geist 焦点环：外侧 2px 背景色间隔 + 2px 焦点色
void FocusRing(ImDrawList* drawList, ImVec2 min, ImVec2 max, float rounding);
// Geist 圆形 Spinner。diameter 为 1x 设计值，12 / 16 / 20 三档用 Geist 原值，其他直径按 20 那档等比缩放。
// 内部调用 RequestRedraw()，保证转动时持续出帧
void Spinner(ImDrawList* drawList, ImVec2 center, float diameter, ImU32 color);

} // namespace SoGui
