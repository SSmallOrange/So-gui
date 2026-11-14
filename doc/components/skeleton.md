# So-gui Skeleton 组件规格

对应 Geist [Skeleton](https://vercel.com/geist/skeleton)。数据来源：2026-11-14 用浏览器自动化抓取官网 demo 元素的 class、计算样式与 keyframes，原始数据在 `doc/specs/skeleton.json`。

## 1. Geist 规格

### 1.1 形状（4 种 variant）

| variant | 圆角 | 出处 |
|---|---|---|
| default | 5px（`rounded-[5px]`） | 全部 demo |
| pill | 全圆角（`rounded-full`，= 短边一半） | `48×24`、`48×48` demo |
| rounded | 与 default 相同的 5px | 官网文档列出的 variant，页面 demo 中与 default 同值 |
| squared | 0（`rounded-none`） | `48×48` demo |

官网对 variant 的描述：default（通用块级）、pill（头像）、rounded（按钮/chips）、squared（图片 tile）。

### 1.2 尺寸

无预设尺寸枚举，由使用者指定 `width` / `height`（So-gui 用 `ImVec2 size` 参数）。官网建议形状和尺寸与最终内容一致，避免加载后布局跳动。

### 1.3 shimmer 动画

- 结构：本体透明，`::after` 伪元素铺一层从左到右的渐变带并循环扫过。
- 渐变（亮色）：`from-gray-100 via-gray-200 to-gray-100`，即 hsla(0,0%,95%) → hsla(0,0%,92%) → hsla(0,0%,95%)。
- 渐变（暗色）：同名色阶，实测 `rgb(26,26,26) → rgb(31,31,31) → rgb(26,26,26)`。
- 动画：`loading-skeleton 1.5s ease-in-out infinite`，keyframes 只有一帧 `100% { transform: translate(-50%) }`。`::after` 初始 `right: -200%`（宽度为元素的 300%），渐变图像占 after 宽度的 50%（`bg-[length:50%_100%]`），一个周期内 after 左移自身宽度的一半，视觉效果是亮带从右往左扫过骨架。
- `animate = false`（官网 `after:animate-none`）时只显示静态底色。

> ImGui 无 CSS 渐变，用 `AddRectFilledMultiColor` 分段近似：把骨架矩形分成 2~4 段，段边界颜色按 `gray-100 → gray-200 → gray-100` 插值，亮带宽度 = 骨架宽 × 1.5，位置随 `ImGui::GetTime()` 循环。静态时底色 `gray-200`。

## 2. API

```cpp
// src/sogui/widgets/skeleton.h
namespace SoGui {

enum class SkeletonVariant { Default, Pill, Rounded, Squared, COUNT };

struct SkeletonOptions {
    SkeletonVariant variant  = SkeletonVariant::Default;
    bool            animate  = true;   // shimmer 扫过动画；关闭时只画静态底色
};

void Skeleton(ImVec2 size, const SkeletonOptions& options = {});

} // namespace SoGui
```

实现要点：

- 非交互，无状态解析。
- 尺寸 = 参数直接指定（1x 设计值经 `Px()` 换算），不自动量文字。
- `animate` 时每帧 `RequestRedraw()`；`animate = false` 不请求。
- 颜色：亮暗都用色阶 `Pal(Hue::Gray, 1)` / `Pal(Hue::Gray, 2)`，无亮暗分支。

## 3. 测试

- 4 种 variant 的圆角（5px / 短边一半 / 5px / 0）。
- `animate = true` 时每帧请求重绘；false 时不请求。
- 顶点颜色落在 gray-100 与 gray-200 之间（亮暗各自验证）。

## 4. 验收

- 4 种形状与官网截图一致；shimmer 视觉节奏接近官网（1.5s 一个周期）。
