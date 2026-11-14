# So-gui Separator 组件规格

对应 Geist [Separator](https://vercel.com/geist/separator)。数据来源：2026-11-14 用浏览器自动化抓取官网 demo 元素的 class 与计算样式，原始数据在 `doc/specs/separator.json`。

## 1. Geist 规格

| 属性 | 值 | 出处 |
|---|---|---|
| 方向 | 仅横向 | 官网 demo 只展示横向 |
| 高度 | 1px（`h-px`） | class `bg-gray-200 shrink-0 h-px w-full` |
| 宽度 | 撑满可用宽度（`w-full`） | 同上 |
| 颜色（亮色） | `gray-200` = hsla(0, 0%, 92%) = `#ebebeb` | class `bg-gray-200` + computed |
| 颜色（暗色） | `gray-200` = hsla(0, 0%, 12%) = `#1f1f1f` | dark 主题 computed |

亮暗两色都是 `gray-200` 色阶（色阶本身随主题变化），实现时直接用 `Pal(Hue::Gray, 2)`，无亮暗分支。

官网没有纵向 Separator（Geist 不提供），第一版不做。纵向分隔的等价物（`ImGui::Separator` 的水平方向）留给使用者自行处理。

## 2. API

```cpp
// src/sogui/widgets/separator.h
namespace SoGui {
// 横向 1px 分隔线，撑满当前可用宽度，颜色 gray-200
void Separator();
} // namespace SoGui
```

实现约 10 行：`ItemSize` 占位 1px 高、`ItemAdd`、`RectFilled` 画 1px 线（颜色经 `Px()` / 像素对齐，保证非整数缩放下不发虚）。

## 3. 测试

- 高度为 1px（`FontScaleMain = 2` 时为 2px）。
- 颜色 = `GetPaletteColor(Hue::Gray, 2)`。
- 宽度 = 可用宽度。

## 4. 验收

- 亮暗主题下与官网截图一致（颜色、1px 线）。
- 150% 缩放下线条仍为清晰整数物理像素。
