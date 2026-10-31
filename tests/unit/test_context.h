#pragma once

#include <sogui/sogui.h>

struct ThemeFixture {
    ImGuiContext* previousImGui = ImGui::GetCurrentContext();
    SoGui::Context* previousSoGui = SoGui::GetCurrentContext();
    ImGuiContext* imguiContext = nullptr;
    SoGui::Context* soguiContext = nullptr;
    bool inFrame = false;

    explicit ThemeFixture(const SoGui::ContextConfig& config = {}) {
        imguiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(imguiContext);
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.DisplaySize = ImVec2(1280.0f, 960.0f);
        io.DeltaTime = 1.0f / 60.0f;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
        soguiContext = SoGui::CreateContext(config);
        SoGui::SetCurrentContext(soguiContext);
    }

    ~ThemeFixture() {
        ImGui::SetCurrentContext(imguiContext);
        SoGui::SetCurrentContext(soguiContext);
        if (inFrame)
            EndFrame();
        SoGui::DestroyContext(soguiContext);
        ImGui::DestroyContext(imguiContext);
        SoGui::SetCurrentContext(previousSoGui);
        ImGui::SetCurrentContext(previousImGui);
    }

    ThemeFixture(const ThemeFixture&) = delete;
    ThemeFixture& operator=(const ThemeFixture&) = delete;

    void BeginFrame() {
        ImGui::NewFrame();
        SoGui::NewFrame();
        inFrame = true;
    }

    void EndFrame() {
        SoGui::EndFrame();
        ImGui::EndFrame();
        inFrame = false;
    }
};
