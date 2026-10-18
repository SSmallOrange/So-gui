#pragma once

#include <imgui.h>

#include <atomic>

namespace SoGui {

struct ContextConfig {};

struct Context {
    unsigned long long frameCount = 0;
    std::atomic<bool>  redrawRequested{false};
};

Context* CreateContext(const ContextConfig& = {});
void     DestroyContext(Context* ctx = nullptr);
Context* GetCurrentContext();
void     SetCurrentContext(Context* ctx);
void     NewFrame();
void     EndFrame();
void     RequestRedraw();
bool     ConsumeRedrawRequest();

inline Context* g_currentContext = nullptr;

inline Context* CreateContext(const ContextConfig&) {
    DestroyContext();
    Context* ctx = IM_NEW(Context)();
    g_currentContext = ctx;
    return ctx;
}

inline void DestroyContext(Context* ctx) {
    if (ctx == nullptr)
        ctx = g_currentContext;
    if (ctx == g_currentContext)
        g_currentContext = nullptr;
    IM_DELETE(ctx);
}

inline Context* GetCurrentContext() { return g_currentContext; }

inline void SetCurrentContext(Context* ctx) { g_currentContext = ctx; }

inline void NewFrame() {
    Context* ctx = GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "SoGui::CreateContext() must be called before SoGui::NewFrame()");
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr &&
              "ImGui::CreateContext() must be called before SoGui::NewFrame()");
    ctx->frameCount++;
}

inline void EndFrame() {
    Context* ctx = GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "SoGui::EndFrame() called without SoGui::NewFrame()");
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "ImGui context missing in SoGui::EndFrame()");
    (void)ctx;
}

inline void RequestRedraw() {
    if (Context* ctx = GetCurrentContext())
        ctx->redrawRequested.store(true);
}

inline bool ConsumeRedrawRequest() {
    Context* ctx = GetCurrentContext();
    if (ctx == nullptr)
        return false;
    return ctx->redrawRequested.exchange(false);
}

} // namespace SoGui
