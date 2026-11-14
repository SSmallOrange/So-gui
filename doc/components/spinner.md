# So-gui Spinner 组件规格

对应 Geist [Spinner](https://vercel.com/geist/spinner)。数据来源：2026-11-14 用浏览器自动化抓取官网 demo 元素的 class、计算样式与 keyframes，原始数据在 `doc/specs/spinner.json`。

## 1. Geist 规格

### 1.1 尺寸（7 档）

官网以 `size-3` ~ `size-14`（Tailwind 尺寸档）给出 7 个直径。每档由 N 根短条组成，短条绕圆心均匀分布，第 0 根指向正右（0°），角度按顺时针增加，每根间隔 `360° / N`。

| 直径 | 条数 | 条长 × 条宽 | 条中心到圆心距离（条长 × 1.46） | 透明度循环周期 |
|---|---|---|---|---|
| 12（size-3） | 8 | 3 × 1.5 | 4.38 | 1.0s |
| 16（size-4） | 10 | 4 × 1.5 | 5.84 | 1.0s |
| 20（size-5） | 12 | 5 × 2 | 7.3 | 1.2s |
| 24（size-6） | 12 | 6 × 2.5 | 8.76 | 1.2s |
| 32（size-8） | 15 | 8 × 2.5 | 11.68 | 1.2s |
| 40（size-10） | 15 | 10 × 3 | 14.6 | 1.2s |
| 56（size-14） | 18 | 14 × 3.5 | 20.44 | 1.3s |

> 出处：demo 元素为 `text-gray-700 relative inline-block aspect-square transform-gpu size-N`，短条为子元素 `absolute ... rounded-full bg-current h-[?px] w-[?px]`，实测尺寸见 `doc/specs/spinner.json`。周期 = 短条 `animation-delay` 的跨度（`animation: spinner-opacity <周期> linear infinite`）。

> **与 Button 方案的差异**：Button 方案（2026-11-01 抓取）记录的是 8 根 / 45° / 1s，官网现已改为按直径变化的条数（12px 档 8 根、20px 档 12 根等）和 1.0~1.3s 的周期。本次按官网新版实现，`draw.cc` 的 `Spinner` 绘制函数同步升级，Button 的 loading 状态自动跟随。

### 1.2 透明度动画

`@keyframes spinner-opacity { 0% { opacity: 1; } 100% { opacity: 0.15; } }`，线性循环。第 i 根短条的延迟为 `-(周期 - 周期/N × i)`（即均匀错开一个相位），第 i 根在时刻 t 的透明度 = `1 − 0.85 × frac(t / 周期 + 1 − i / N)`。

### 1.3 颜色（4 种）

| 颜色 | 官网 class | So-gui Token |
|---|---|---|
| default | `text-gray-700` | `Sem(Col::TextDisabled)`（gray-700，亮暗同值） |
| red | `text-red-700` | `Pal(Hue::Red, 7)` |
| green | `text-green-700` | `Pal(Hue::Green, 7)` |
| blue | `text-blue-700` | `Pal(Hue::Blue, 7)` |

短条用 `bg-current`（currentColor），颜色由容器决定。

## 2. API

```cpp
// src/sogui/widgets/spinner.h
namespace SoGui {

enum class SpinnerColor { Default, Red, Green, Blue, COUNT };

struct SpinnerOptions {
    SpinnerColor color = SpinnerColor::Default;
};

// 在当前位置画一个 Spinner。diameter 为 1x 设计值（CSS px），
// 12 / 16 / 20 / 24 / 32 / 40 / 56 七档用官网原值，其他直径按 20 那档等比缩放。
// 内部调用 RequestRedraw()，转动期间持续出帧。
void Spinner(float diameter, const SpinnerOptions& options = {});

} // namespace SoGui
```

`draw.h` 中现有的绘制函数 `Spinner(ImDrawList*, ImVec2 center, float diameter, ImU32 color)` 升级为新规格并改名为 `SpinnerDraw`，组件与 Button loading 都调用它。

## 3. 测试

- 三档（12 / 20 / 32）的短条数与 `doc/specs/spinner.json` 一致。
- 短条颜色：default = `TextDisabled`；red/green/blue = 对应色阶 7。
- 无窗口跑帧时，每帧都请求重绘（`ConsumeRedrawRequest()` 为 true）。
- 任意直径（如 28）不越界、短条数 > 0。

## 4. 验收

- 三档尺寸与官网截图一致；default 色在亮暗主题下都是 `gray-700`。
- Demo 页 7 档全部展示，转动流畅，页面静止时帧计数仍在增长（Spinner 本身要求持续出帧）。
