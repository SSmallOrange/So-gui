#pragma once

#include <imgui.h>

namespace SoGui {

enum class ButtonVariant { Primary, Secondary, Tertiary, Error, Warning, COUNT };
enum class ButtonSize { Small, Medium, Large, COUNT };

struct ButtonOptions {
    ButtonVariant variant   = ButtonVariant::Primary;
    ButtonSize    size      = ButtonSize::Medium;
    bool          loading   = false;   // 显示 Spinner，外观和行为同禁用
    bool          disabled  = false;
    bool          fullWidth = false;   // 宽度撑满当前可用区域
};

// 被点击（鼠标松开，或键盘按空格 / Enter）的那一帧返回 true。
// label 支持 ImGui 的 "##" 写法：只显示 "##" 之前的部分，整个字符串参与生成 ID。
bool Button(const char* label, const ButtonOptions& options = {});

} // namespace SoGui
