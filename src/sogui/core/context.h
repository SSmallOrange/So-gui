#pragma once

#include <sogui/core/theme.h>

namespace SoGui {

struct ContextConfig {
    ThemeId theme = ThemeId::Light;
    bool    applyToImGuiStyle = true;  // 在已使用 ImGuiStyle 时，决定是否启用 So-Gui
};

struct Context; // 定义 sogui_internal.h

Context* CreateContext(const ContextConfig& config = {});
void     DestroyContext(Context* ctx = nullptr);
Context* GetCurrentContext();
void     SetCurrentContext(Context* ctx);
void     NewFrame();
void     EndFrame();
void     RequestRedraw();
bool     ConsumeRedrawRequest();

} // namespace SoGui
