#pragma once

namespace SoGui {

enum class SpinnerColor { Default, Red, Green, Blue, COUNT };

struct SpinnerOptions {
    SpinnerColor color = SpinnerColor::Default;
};

// 在当前位置画一个 Spinner。diameter 为 1x 设计值（CSS px），
// 12 / 16 / 20 / 24 / 32 / 40 / 56 七档用官网原值，其他直径按 20 那档等比缩放。
// 转动期间每帧调用 RequestRedraw()。
void Spinner(float diameter, const SpinnerOptions& options = {});

} // namespace SoGui
