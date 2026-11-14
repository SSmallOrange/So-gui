#pragma once

#include <imgui.h>

namespace SoGui {

enum class SkeletonVariant { Default, Pill, Rounded, Squared, COUNT };

struct SkeletonOptions {
    SkeletonVariant variant = SkeletonVariant::Default;
    bool            animate = true; // shimmer 扫过动画；关闭时只画静态底色
};

// 尺寸由使用方指定（1x 设计值，经 Px() 换算），形状应与最终内容一致。
void Skeleton(ImVec2 size, const SkeletonOptions& options = {});

} // namespace SoGui
