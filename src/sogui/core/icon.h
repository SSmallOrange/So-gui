#pragma once

#include <sogui/core/theme.h>

namespace SoGui {

#include "generated/icons_lucide.inc"

// 在当前行画一个图标。size 为 1x 设计值（CSS px），颜色默认 TextPrimary。
void Icon(IconId id, float size = 14.0f, Col color = Col::TextPrimary);

} // namespace SoGui
