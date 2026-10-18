#pragma once

namespace SoGui {

struct ContextConfig {};

struct Context;

Context* CreateContext(const ContextConfig& config = {});
void     DestroyContext(Context* ctx = nullptr);
Context* GetCurrentContext();
void     SetCurrentContext(Context* ctx);
void     NewFrame();
void     EndFrame();
void     RequestRedraw();
bool     ConsumeRedrawRequest();

} // namespace SoGui
