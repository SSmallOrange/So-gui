# So-gui Badge 组件规格

对应 Geist [Badge](https://vercel.com/geist/badge)。数据来源：2026-11-14 用浏览器自动化抓取官网 demo 元素的 class 与计算样式，原始数据在 `doc/specs/badge.json`。

## 1. Geist 规格

### 1.1 尺寸（3 档）

| 尺寸 | 高度 | 水平内边距 | 垂直内边距 | 字号 / 行高 | 图标 |
|---|---|---|---|---|---|
| Small | 20 | 8（`px-2`） | 2（`py-0.5`） | 11 / 20 | 12（`size-3`） |
| Medium（默认） | 24 | 12（`px-3`） | 2 | 12 / 24 | 14（`size-3.5`） |
| Large | 32 | 12 | 2 | 14 / 20（`text-sm`） | 16（`size-4`） |

> 出处：demo class `h-6 px-3 gap-1.25 text-[12px]/[24px]`（Medium）、`h-5 px-2 text-[11px]/[20px]`（Small）、`h-8 px-3 text-sm`（Large）；图标 `**:data-[slot=icon]:size-3.5`、`-ml-1`（图标距文字 -4px，即 gap 5px − 4px 偏移）。

- 圆角：胶囊形（`rounded-full`，实际圆角 = 高度 / 2）。
- 字重 500（`font-medium`）、`tabular-nums`（等宽数字，ImGui 无此能力，忽略）。
- 图标与文字的间距 5px（`gap-1.25`）；有图标时图标再向左 -4px（`-ml-1`）。

### 1.2 颜色（17 种 variant）

颜色类出自官网 demo class（`bg-(--ds-xxx)` / `text-(--ds-xxx)` 语法），亮暗两套：

| variant | 亮色背景 | 暗色背景 | 文字 |
|---|---|---|---|
| gray | `gray-900` | `gray-500` | `Bg(1)` |
| gray-subtle | 透明 | 透明 | `gray-1000` |
| blue | `blue-800` | `blue-800` | `Bg(1)` |
| blue-subtle | 透明 | 透明 | `blue-900` |
| purple | `purple-900` | `purple-500` | `Bg(1)` |
| purple-subtle | 透明 | 透明 | `purple-900` |
| amber | `amber-700` | `amber-700` | `Lit(0x000000)`（黑字） |
| amber-subtle | 透明 | 透明 | `amber-900` |
| red | `red-900` | `red-800` | `Bg(1)` |
| red-subtle | 透明 | 透明 | `red-900` |
| pink | `pink-900` | `pink-600` | `Bg(1)` |
| pink-subtle | 透明 | 透明 | `pink-900` |
| green | `green-900` | `green-600` | `Bg(1)` |
| green-subtle | 透明 | 透明 | `green-900` |
| teal | `teal-900` | `teal-600` | `Bg(1)` |
| teal-subtle | 透明 | 透明 | `teal-900` |
| inverted | `gray-1000` | `gray-1000` | `gray-100` |

> 出处：`bg-(--ds-gray-900) dark-theme:bg-(--ds-gray-500) text-(--ds-contrast-fg)` 等（见 `doc/specs/badge.json` 中各元素的 class）。`gray-subtle` 的暗色文字 = `gray-1000`（暗色下为 #ededed），与亮色同名色阶，无需亮暗分支。
> subtle 变体无边框（官网 demo computed box-shadow: none）。

### 1.3 交互状态

官网 Badge 无 hover / active 颜色变化（73 个样例实测无变化，2026-11-14 抓取），So-gui 不实现状态过渡。

`icon`（图标 + 文本组合）在 Lucide 图标体系接入后的里程碑实现。

## 2. API

```cpp
// src/sogui/widgets/badge.h
namespace SoGui {

enum class BadgeVariant {
    Gray, GraySubtle, Blue, BlueSubtle, Purple, PurpleSubtle, Amber, AmberSubtle,
    Red, RedSubtle, Pink, PinkSubtle, Green, GreenSubtle, Teal, TealSubtle, Inverted,
    COUNT
};
enum class BadgeSize { Small, Medium, Large, COUNT };

struct BadgeOptions {
    BadgeVariant variant = BadgeVariant::Gray;
    BadgeSize    size    = BadgeSize::Medium;
};

void Badge(std::string_view label, const BadgeOptions& options = {});

} // namespace SoGui
```

实现要点：

- 非交互组件，不做 `ResolveInteraction` / 动画 / 焦点。
- 布局：高度按 1.1 节；宽度 = 左右内边距 × 2 + 文字宽。圆角 = 高度 / 2。
- 文字：`FontWeight::Medium` + 任意字号（11 / 12 / 14px 不在 TextStyle 表里，字号写在组件 Token 表并注明出处），行盒按 CSS half-leading 计算基线。
- 文字与按钮等同行混排时基线对齐，复用 Button 的基线算法。

## 3. 测试

- 三档尺寸：高度 20 / 24 / 32（`FontScaleMain = 2` 时翻倍）。
- 17 种 variant × 亮暗：背景、文字颜色与 1.2 节表格一致。
- subtle 变体无边框顶点；其他变体无边框。
- 宽度 = 文字宽 + 2 × 内边距。

## 4. 验收

- 17 变体 × 3 尺寸与官网并排对比一致（亮暗各一次）。
- 中文标签垂直居中。
