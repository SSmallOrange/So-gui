#include <sogui/sogui_internal.h>

#include <imgui.h>

namespace SoGui {

static Context* g_currentContext = nullptr;

Context* CreateContext(const ContextConfig& config) {
    Context* ctx = IM_NEW(Context)();
    ThemeInitialize(ctx->theme, config);
    if (g_currentContext == nullptr)
        SetCurrentContext(ctx);
    return ctx;
}

void DestroyContext(Context* ctx) {
    if (ctx == nullptr)
        ctx = g_currentContext;
    if (ctx == g_currentContext)
        SetCurrentContext(nullptr);
    IM_DELETE(ctx);
}

Context* GetCurrentContext() { return g_currentContext; }

void SetCurrentContext(Context* ctx) { g_currentContext = ctx; }

void NewFrame() {
    Context* ctx = GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "SoGui::CreateContext() must be called before SoGui::NewFrame()");
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr &&
              "ImGui::CreateContext() must be called before SoGui::NewFrame()");
    ctx->frameCount++;
    ThemeNewFrame(ctx->theme);
}

void EndFrame() {
    Context* ctx = GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "SoGui::EndFrame() called without SoGui::NewFrame()");
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "ImGui context missing in SoGui::EndFrame()");
    ThemeEndFrame(ctx->theme);
}

void RequestRedraw() {
    if (Context* ctx = GetCurrentContext())
        ctx->redrawRequested.store(true);
}

bool ConsumeRedrawRequest() {
    Context* ctx = GetCurrentContext();
    if (ctx == nullptr)
        return false;
    return ctx->redrawRequested.exchange(false);
}

} // namespace SoGui
