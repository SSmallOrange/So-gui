#pragma once

namespace SoGui {

// 设计尺寸使用 1x 逻辑像素，与 ImGui 字号使用同一组缩放系数
float GetScale();
float Px(float value);

} // namespace SoGui
