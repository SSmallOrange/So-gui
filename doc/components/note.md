# So-gui Note 组件规格

对应 Geist [Note](https://vercel.com/geist/note)。数据来源：2026-11-14 用浏览器自动化抓取官网 demo 元素的 class 与计算样式，原始数据在 `doc/specs/note.json`。

## 1. Geist 规格

### 1.1 结构

官网 Note 由三部分组成：图标（变体默认图标）、可选 `label`（1~2 词 Title Case 前缀）、`content`（一句正文）、可选 `action`（内联 CTA）。So-gui 实现 label + content；图标（Lucide）与 action 在后续里程碑实现。

- **label**：官网实测 14px / 600 字重（`Region Change:`，2026-11-14 抓取），对应 `TextStyle::Heading14`。Small 尺寸下官网未提供 label 样例，沿用同一样式。
- **图标**：官网为 16×16 viewBox 单形状 SVG，颜色随 type 主色（default `gray`、success 蓝 `#52a8ff`、error 红 `#ff6166`、warning 琥珀 `#f2a20d`、secondary 白 60%、violet 紫 `#bf7af0`、cyan 青 `#0ac7b4`），显示尺寸 14px。原始形状数据见 `doc/specs/note.json`。

### 1.2 尺寸（2 档）

| 尺寸 | 上下内边距 | 左右内边距 | 圆角 | gap（图标/文字间距） | 文字 |
|---|---|---|---|---|---|
| Small | 6 | 8 | 6（`rounded-md`） | 12（`gap-3`） | 13px |
| Medium（默认） | 6 | 12 | 6 | 12 | 14px / 400 |

> 出处：`px-3 py-1.5 min-h-(--ds-size-medium) text-copy-14`；Small 版实测 `px-2`、字 13px。行高与 `Copy14`（20px）一致，文字撑高时容器跟随（`min-h` 36px = `--ds-size-medium`）。

- 文字行高：官网 `text-copy-14` 行高 20px（与现有 `TextStyle::Copy14` 一致，Small 用 13px 近似 `Copy13`）。
- 长文本自动换行并撑高容器。

### 1.3 颜色（8 种 type）

`type` → 配色，class 出自官网 demo（Tailwind 色阶名，亮暗两套色阶自动随主题变化）：

| type | 描边版（默认） | filled 版 |
|---|---|---|
| default | `border-gray-400` + `text-gray-900`，透明底 | `border-blue-100 bg-blue-200 text-blue-900` |
| success | `border-blue-400` + `text-blue-900` | `border-blue-100 bg-blue-200 text-blue-900` |
| error | `border-red-400` + `text-red-900` | `border-red-100 bg-red-200 text-red-900` |
| warning | `border-amber-400` + `text-amber-900` | `border-amber-100 bg-amber-200 text-amber-900` |
| secondary | `border-transparent bg-gray-alpha-200` + `text-gray-alpha-900` | 同左（无 filled 变化） |
| violet | `border-purple-400` + `text-purple-900` | `border-purple-100 bg-purple-200 text-purple-900` |
| cyan | `border-teal-400` + `text-teal-900` | `border-teal-100 bg-teal-200 text-teal-900` |
| disabled | 同 default（配色不变，仅交互禁用） | — |

> 注意：官网的 success 用蓝色系（`border-blue-400 text-blue-900`），并非绿色——已通过官网 Success 区块实测确认。
> 边框画在容器内侧（`border` + `box-border`，与 Button 的 box-shadow 外描边不同），占布局空间。disabled 第一版暂不做（Note 无交互，语义上等同 default，官网也未改变外观）。

### 1.4 交互状态

官网 Note 无 hover / active 颜色变化（73 个样例实测无变化，2026-11-14 抓取），So-gui 不实现状态过渡。

官网 `variant="info"` 不存在，default 即信息样式。

## 2. API

```cpp
// src/sogui/widgets/note.h
namespace SoGui {

enum class NoteType { Default, Success, Error, Warning, Secondary, Violet, Cyan, COUNT };
enum class NoteSize { Small, Medium, COUNT };

struct NoteOptions {
    NoteType type   = NoteType::Default;
    NoteSize size   = NoteSize::Medium;
    bool     filled = false;   // 填充底色（Secondary 无效果）
};

// label 可为空（空串时只显示 content）；content 自动换行
void Note(std::string_view label, std::string_view content, const NoteOptions& options = {});

} // namespace SoGui
```

实现要点：

- 非交互，无状态解析/动画/焦点。
- 布局：label 用 `Heading14`（14px SemiBold）+ content 用 Copy13/Copy14（Regular），label 与 content 间无额外间距（同 gap 12px）；宽度默认撑满可用宽度（`w-full`），单行时内容高度 20px。
- 边框在容器内侧（`RectRing` 的 offset 为负…… ImGui 无内侧描边 API，改用 `AddRect` 内缩 0.5px 或画在圆角矩形上，坐标按像素对齐），filled 时边框为背景色系浅一档。
- 文字色：label 与 content 同色（官网 label 同 content 颜色）。

## 3. 测试

- 两种尺寸：Medium 高度 ≥ 36（`min-h`），Small ≥ 32；padding 与 1.2 节一致。
- 8 种 type × filled × 亮暗：背景、边框、文字颜色与 1.3 节一致。
- 长文本换行后容器高度增加。
- 空 label 时不渲染 label，高度不变。

## 4. 验收

- 全部 type 与官网并排对比（亮暗 × 描边/filled）。
- 中文 content 换行正常，行高与 `SoGui::Text(Copy14)` 一致。
