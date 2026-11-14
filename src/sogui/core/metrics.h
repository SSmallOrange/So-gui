#pragma once

namespace SoGui::Metrics {

// 1x 设计值（CSS px），使用时经过 Px() 换算
inline constexpr float kControlHeightSmall  = 32.0f;  // --ds-size-small
inline constexpr float kControlHeightMedium = 36.0f;  // --ds-size-medium
inline constexpr float kControlHeightLarge  = 40.0f;  // --ds-size-large
inline constexpr float kBorderWidth         = 1.0f;   // box-shadow: 0 0 0 1px
inline constexpr float kFocusRingGap        = 2.0f;   // --ds-focus-ring 第一层：0 0 0 2px 背景色
inline constexpr float kFocusRingWidth      = 2.0f;   // --ds-focus-ring 第二层：0 0 0 4px 焦点色，去掉间隔后宽 2px

} // namespace SoGui::Metrics
