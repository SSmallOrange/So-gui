# Button

对应 Geist 的 [Button](https://vercel.com/geist/button)。数值以 2026-11-01 抓取的官网 CSS 为准。

## 接口

```cpp
enum class ButtonVariant { Primary, Secondary, Tertiary, Error, Warning, COUNT };
enum class ButtonSize { Small, Medium, Large, COUNT };

struct ButtonOptions {
    ButtonVariant variant   = ButtonVariant::Primary;
    ButtonSize    size      = ButtonSize::Medium;
    bool          loading   = false;   // 显示 Spinner，外观和行为同禁用
    bool          disabled  = false;
    bool          fullWidth = false;   // 宽度撑满当前可用区域
};

bool Button(const char* label, const ButtonOptions& options = {});
```

`label` 支持 ImGui 的 `##` 写法：只显示 `##` 之前的部分，整个字符串参与生成 ID。被点击（鼠标松开，或键盘按空格 / Enter）的那一帧返回 `true`。

## 尺寸（1x 设计值，绘制时乘 `Px()`）

| 尺寸 | 高度 | 左右内边距 | 文字样式 | 圆角 | Spinner 直径 |
|---|---|---|---|---|---|
| Small | 32 | 6 | Button14 | 6 | 12 |
| Medium（默认） | 36 | 10 | Button14 | 6 | 16 |
| Large | 40 | 14 | Button16 | 8 | 20 |

文字外还有一层 `px-1.5`（左右各 6px）。loading 时 Spinner 在文字左边，占至少 20px 宽的格子，右边留 3px。

## 颜色

按下时和悬停时颜色相同（Geist 定义了 `--themed-press-bg` 但没有使用），Token 表保留 `active` 一列，目前填悬停色。

**Primary**

| 部位 | 普通 | 悬停 / 按下 |
|---|---|---|
| 背景 | `gray-1000` | 亮 `#383838`，暗 `#CCCCCC` |
| 文字 | `background-100` | 同左 |
| 边框 | 无 | 无 |

**Secondary**

| 部位 | 普通 | 悬停 / 按下 |
|---|---|---|
| 背景 | `background-100` | 亮 `gray-100`，暗 `gray-200` |
| 文字 | `gray-1000` | 同左 |
| 边框 1px | `gray-400` | `gray-alpha-200` |

**Tertiary**

| 部位 | 普通 | 悬停 / 按下 |
|---|---|---|
| 背景 | 透明 | `gray-alpha-200` |
| 文字 | `gray-1000` | 同左 |
| 边框 | 无 | 无 |

**Error**

| 部位 | 普通 | 悬停 / 按下 |
|---|---|---|
| 背景 | `red-800` | 亮 `#AE292F`，暗 `red-900` |
| 文字 | 亮 `#F5F5F5`，暗 `#FFFFFF` | 同左 |
| 边框 | 无 | 无 |

**Warning**（亮暗相同）

| 部位 | 普通 | 悬停 / 按下 |
|---|---|---|
| 背景 | `amber-800` | `#D27504` |
| 文字 | `#0A0A0A` | 同左 |
| 边框 | 无 | 无 |

**禁用 / loading**

| 部位 | Primary、Secondary、Error、Warning | Tertiary |
|---|---|---|
| 背景 | `gray-100` | 透明 |
| 文字 | `gray-700` | `gray-700`（与 Geist 不同，见下） |
| 边框 1px | `gray-400` | 无 |

**焦点环**：键盘（Tab）选中时显示，鼠标点击后消失。紧贴按钮外侧先留 2px 背景色间隔，再往外画 2px 焦点色（亮 `blue-700` / 暗 `blue-900`）。

## 动画

背景、文字、边框色过渡 150ms `ease-in-out`；焦点环不做过渡。

## Spinner

8 根圆头短条沿半径均匀排布，1s 内透明度从 1 线性降到 0.15 后循环，相邻错开 125ms。颜色 `gray-700`。绘制函数在 `draw.h` 的 `Spinner`，以后作为独立组件。

## 已实现

- 5 种变体、3 种尺寸、`disabled`、`loading`、`fullWidth`
- 亮 / 暗两套配色，悬停 / 按下 / 禁用 / 焦点环
- 150ms 颜色过渡、loading Spinner 动画
- 与原生 `ImGui::Text`、`SoGui::Text` 的基线对齐
- 禁用时抵消 ImGui 的 `DisabledAlpha`，用 Token 里的禁用色

## 暂未实现

- `prefix` / `suffix` 前后图标（等 Lucide 图标接入）
- `svgOnly` + `shape`（square / circle）纯图标按钮
- `shape="rounded"` + 阴影（等 Material 阴影）

## 不实现

- `ButtonLink`（桌面应用没有 URL 导航）
- `CustomButton`（自定义三态颜色，以后单独设计）
- `typeName`、`aria-label`（Web 专属 HTML 属性）

## 与 Geist 不同的地方

1. **禁用的 Tertiary 文字**：Geist 的 Tertiary 文字色带 `!important`，盖过了 `disabled` 的 `gray-700`，导致官网禁用的 Tertiary 和正常态长得一样。So-gui 按本意处理，禁用文字用 `gray-700`。
2. **loading 时不能用键盘选中**：Geist 要求 loading 按钮仍可聚焦，但 ImGui 里禁用的控件不能被导航到。So-gui 沿用 ImGui 行为。
3. **边框画在控件外侧**：Geist 用 `box-shadow: 0 0 0 1px` 画边框，不占布局空间，So-gui 照搬。

## 已知偏差

- 抗锯齿让直边略软（约 1px），浏览器直边是硬的。
- 文字宽度约有 1% 误差（ImGui 字号取整），按钮宽度可能和官网差 1px。
