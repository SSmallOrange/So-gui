#pragma once

#include <sogui/core/theme.h>

#include <imgui.h>

#include <string_view>

namespace SoGui {

enum class TextStyle {
    Heading72, Heading64, Heading56, Heading48, Heading40, Heading32, Heading24, Heading20, Heading16, Heading14,
    Button16, Button14, Button12,
    Label20, Label18, Label16, Label14, Label14Mono, Label13, Label13Mono, Label12, Label12Mono,
    Copy24, Copy20, Copy18, Copy16, Copy14, Copy13, Copy13Mono,
    COUNT
};

struct TextOptions {
    Col color = Col::TextPrimary;
};

struct TextMetrics {
    float fontSize = 0.0f; // 当前 em 字号
    float lineHeight = 0.0f;
    float baseline = 0.0f; // 行顶到基线
};

void Text(std::string_view text, TextStyle style = TextStyle::Copy14, const TextOptions& options = {});
// wrapWidth <= 0 时不自动换行
ImVec2 CalcTextSize(std::string_view text, TextStyle style, float wrapWidth = 0.0f);
TextMetrics GetTextMetrics(TextStyle style);
const char* GetTextStyleName(TextStyle style);

} // namespace SoGui
