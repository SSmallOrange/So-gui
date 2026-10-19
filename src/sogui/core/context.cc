#include <sogui/sogui_internal.h>

#include <imgui.h>

namespace SoGui {

static Context* g_currentContext = nullptr;

Context* CreateContext(const ContextConfig&) {
    DestroyContext();
    Context* ctx = IM_NEW(Context)();
    g_currentContext = ctx;
    return ctx;
}

void DestroyContext(Context* ctx) {
    if (ctx == nullptr)
        ctx = g_currentContext;
    if (ctx == g_currentContext)
        g_currentContext = nullptr;
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
}

void EndFrame() {
    Context* ctx = GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "SoGui::EndFrame() called without SoGui::NewFrame()");
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "ImGui context missing in SoGui::EndFrame()");
    (void)ctx;
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
