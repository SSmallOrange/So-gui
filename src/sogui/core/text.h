#pragma once

#include <sogui/core/theme.h>

#include <imgui.h>

#include <string_view>

namespace SoGui {

enum class TextStyle { Label14, Copy14, Heading24, COUNT };

struct TextOptions {
    Col color = Col::TextPrimary;
};

struct TextMetrics {
    float fontSize = 0.0f; // 当前 em 字号
    float lineHeight = 0.0f;
    float baseline = 0.0f; // 行顶到基线
};

// 以下函数需在 ImGui 窗口内调用。
// Label 单行，Copy 和 Heading 按可用宽度换行；都保留显式换行。
void Text(std::string_view text, TextStyle style = TextStyle::Copy14, const TextOptions& options = {});
// wrapWidth <= 0 时不自动换行
ImVec2 CalcTextSize(std::string_view text, TextStyle style, float wrapWidth = 0.0f);
TextMetrics GetTextMetrics(TextStyle style);
const char* GetTextStyleName(TextStyle style);

} // namespace SoGui
